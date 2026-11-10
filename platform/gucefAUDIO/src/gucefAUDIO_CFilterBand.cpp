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

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#include "gucefAUDIO_CFilterBand.h"

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

const Float64 GlideSeconds = 0.020;
const Float64 CrossfadeSeconds = 0.010;

/** Q of the two sections of a fourth order Butterworth response: 1 / ( 2 cos( pi / 8 ) ) and 1 / ( 2 cos( 3 pi / 8 ) ) */
const Float64 ButterworthQ1 = 0.5411961001461970;
const Float64 ButterworthQ2 = 1.3065629648763766;

const UInt32 SteepFlag = 8;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFilterBand::CFilterBand( void )
    : m_live( 0 )
    , m_fading( false )
    , m_fadeLeft( 0 )
    , m_fadeLength( 480 )
    , m_wanted( -1 )
    , m_frequency()
    , m_q()
    , m_gain()
    , m_channels( 0 )
    , m_sampleRate( 48000.0 )
{GUCEF_TRACE;

    m_configuration[ 0 ] = -1;
    m_configuration[ 1 ] = -1;
}

/*-------------------------------------------------------------------------*/

CFilterBand::~CFilterBand()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char* const*
CFilterBand::GetTypeLabels( void )
{GUCEF_TRACE;

    static const char* const labels[ FILTERBANDTYPE_COUNT ] =
    {
        "peak", "lowShelf", "highShelf", "lowPass", "highPass", "bandPass", "notch", "allPass"
    };
    return labels;
}

/*-------------------------------------------------------------------------*/

TFilterResponse
CFilterBand::GetResponse( TFilterBandType type )
{
    // no GUCEF_TRACE: called on the audio thread

    switch ( type )
    {
        case FILTERBANDTYPE_PEAK :      return FILTERRESPONSE_PEAKING;
        case FILTERBANDTYPE_LOWSHELF :  return FILTERRESPONSE_LOWSHELF;
        case FILTERBANDTYPE_HIGHSHELF : return FILTERRESPONSE_HIGHSHELF;
        case FILTERBANDTYPE_LOWPASS :   return FILTERRESPONSE_LOWPASS;
        case FILTERBANDTYPE_HIGHPASS :  return FILTERRESPONSE_HIGHPASS;
        case FILTERBANDTYPE_BANDPASS :  return FILTERRESPONSE_BANDPASS_PEAK0DB;
        case FILTERBANDTYPE_NOTCH :     return FILTERRESPONSE_NOTCH;
        default :                       return FILTERRESPONSE_ALLPASS;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CFilterBand::ConfigurationOf( bool enabled, TFilterBandType type, bool steep )
{
    if ( !enabled )
        return -1;
    return (Int32) type + ( steep ? (Int32) SteepFlag : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CFilterBand::Prepare( UInt32 channelCount, Float64 sampleRate )
{GUCEF_TRACE;

    if ( 0 == channelCount || !( sampleRate > 0.0 ) )
        return false;
    m_channels = channelCount;
    m_sampleRate = sampleRate;
    for ( UInt32 slot=0; slot<2; ++slot )
        for ( UInt32 stage=0; stage<2; ++stage )
            if ( !m_filters[ slot ][ stage ].Prepare( channelCount ) )
                return false;

    m_frequency.Prepare( AUDIOSMOOTHING_ONE_POLE, GlideSeconds, sampleRate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_q.Prepare( AUDIOSMOOTHING_ONE_POLE, GlideSeconds, sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_gain.Prepare( AUDIOSMOOTHING_ONE_POLE, GlideSeconds, sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_frequency.Reset( 1000.0f );
    m_q.Reset( 0.7071f );
    m_gain.Reset( 0.0f );

    const Float64 fade = floor( CrossfadeSeconds * sampleRate + 0.5 );
    m_fadeLength = fade >= 1.0 ? (UInt32) fade : 1;
    m_configuration[ 0 ] = -1;
    m_configuration[ 1 ] = -1;
    m_wanted = -1;
    m_live = 0;
    m_fading = false;
    m_fadeLeft = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFilterBand::ApplyCoefficients( UInt32 slot )
{
    const Int32 configuration = m_configuration[ slot ];
    if ( configuration < 0 )
        return;

    const TFilterBandType type = (TFilterBandType) ( configuration & ( SteepFlag - 1 ) );
    const bool steep = configuration >= (Int32) SteepFlag;
    const TFilterResponse response = GetResponse( type );
    const Float64 frequency = (Float64) m_frequency.GetValue();
    const Float64 q = (Float64) m_q.GetValue();
    Float64 gain = (Float64) m_gain.GetValue();

    if ( !steep )
    {
        m_filters[ slot ][ 0 ].SetResponse( response, m_sampleRate, frequency, q, gain );
        return;
    }
    if ( FILTERBANDTYPE_LOWPASS == type || FILTERBANDTYPE_HIGHPASS == type )
    {
        m_filters[ slot ][ 0 ].SetResponse( response, m_sampleRate, frequency, ButterworthQ1, 0.0 );
        m_filters[ slot ][ 1 ].SetResponse( response, m_sampleRate, frequency, ButterworthQ2, 0.0 );
        return;
    }
    if ( FILTERBANDTYPE_PEAK == type || FILTERBANDTYPE_LOWSHELF == type || FILTERBANDTYPE_HIGHSHELF == type )
        gain *= 0.5;
    m_filters[ slot ][ 0 ].SetResponse( response, m_sampleRate, frequency, q, gain );
    m_filters[ slot ][ 1 ].SetResponse( response, m_sampleRate, frequency, q, gain );
}

/*-------------------------------------------------------------------------*/

void
CFilterBand::Reset( bool enabled, TFilterBandType type, bool steep, Float32 frequency, Float32 q, Float32 gainInDecibels )
{
    for ( UInt32 slot=0; slot<2; ++slot )
        for ( UInt32 stage=0; stage<2; ++stage )
            m_filters[ slot ][ stage ].Reset();

    m_frequency.Reset( frequency );
    m_q.Reset( q );
    m_gain.Reset( gainInDecibels );
    m_wanted = ConfigurationOf( enabled, type, steep );
    m_live = 0;
    m_configuration[ 0 ] = m_wanted;
    m_configuration[ 1 ] = -1;
    m_fading = false;
    m_fadeLeft = 0;
    ApplyCoefficients( 0 );
}

/*-------------------------------------------------------------------------*/

void
CFilterBand::SetTarget( bool enabled, TFilterBandType type, bool steep, Float32 frequency, Float32 q, Float32 gainInDecibels )
{
    m_wanted = ConfigurationOf( enabled, type, steep );
    m_frequency.SetTarget( frequency );
    m_q.SetTarget( q );
    m_gain.SetTarget( gainInDecibels );
}

/*-------------------------------------------------------------------------*/

bool
CFilterBand::IsActive( void ) const
{
    return m_fading || m_configuration[ m_live ] >= 0;
}

/*-------------------------------------------------------------------------*/

void
CFilterBand::StepSubBlock( void )
{
    if ( !m_fading && m_wanted == m_configuration[ m_live ] && m_wanted < 0 )
    {
        // off and staying off: only the settings glide on, nothing is filtered
        if ( m_frequency.IsSmoothing() || m_q.IsSmoothing() || m_gain.IsSmoothing() )
        {
            m_frequency.Step();
            m_q.Step();
            m_gain.Step();
        }
        return;
    }

    bool apply = false;
    if ( m_frequency.IsSmoothing() || m_q.IsSmoothing() || m_gain.IsSmoothing() )
    {
        m_frequency.Step();
        m_q.Step();
        m_gain.Step();
        apply = true;
    }

    if ( !m_fading && m_wanted != m_configuration[ m_live ] )
    {
        // from what is playing to what is asked for: the new section starts clean and takes over in 10 ms
        m_live = 1 - m_live;
        m_configuration[ m_live ] = m_wanted;
        m_filters[ m_live ][ 0 ].Reset();
        m_filters[ m_live ][ 1 ].Reset();
        m_fading = true;
        m_fadeLeft = m_fadeLength;
        apply = true;
    }

    if ( apply )
    {
        ApplyCoefficients( m_live );
        if ( m_fading )
            ApplyCoefficients( 1 - m_live );
    }

    for ( UInt32 slot=0; slot<2; ++slot )
    {
        if ( m_configuration[ slot ] >= 0 && ( slot == m_live || m_fading ) )
        {
            m_filters[ slot ][ 0 ].FlushDenormals();
            if ( m_configuration[ slot ] >= (Int32) SteepFlag )
                m_filters[ slot ][ 1 ].FlushDenormals();
        }
    }
}

/*-------------------------------------------------------------------------*/

inline Float32
CFilterBand::RunSlot( UInt32 slot, UInt32 channel, Float32 input )
{
    const Int32 configuration = m_configuration[ slot ];
    if ( configuration < 0 )
        return input;
    const Float32 first = m_filters[ slot ][ 0 ].Process( channel, input );
    if ( configuration >= (Int32) SteepFlag )
        return m_filters[ slot ][ 1 ].Process( channel, first );
    return first;
}

/*-------------------------------------------------------------------------*/

void
CFilterBand::ProcessRun( Float32* interleaved, UInt32 frames, UInt32 passMask )
{
    if ( !m_fading && m_configuration[ m_live ] < 0 )
        return;

    const UInt32 channels = m_channels;

    if ( !m_fading )
    {
        const bool steep = m_configuration[ m_live ] >= (Int32) SteepFlag;
        for ( UInt32 c=0; c<channels; ++c )
        {
            if ( ( passMask >> c ) & 1 )
                continue;
            m_filters[ m_live ][ 0 ].ProcessChannel( c, interleaved + c, interleaved + c, frames, channels );
            if ( steep )
                m_filters[ m_live ][ 1 ].ProcessChannel( c, interleaved + c, interleaved + c, frames, channels );
        }
        return;
    }

    // crossfade: the weight of the new section grows from where it was
    const UInt32 oldSlot = 1 - m_live;
    UInt32 done = 0;
    while ( done < frames )
    {
        const UInt32 run = frames - done > 32 ? 32 : frames - done;
        Float32 weights[ 32 ];
        for ( UInt32 f=0; f<run; ++f )
        {
            const UInt32 inFade = done + f;
            weights[ f ] = inFade < m_fadeLeft ? (Float32) ( m_fadeLength - m_fadeLeft + inFade + 1 ) / (Float32) m_fadeLength : 1.0f;
        }
        for ( UInt32 c=0; c<channels; ++c )
        {
            if ( ( passMask >> c ) & 1 )
                continue;
            Float32* sample = interleaved + (size_t) done * channels + c;
            for ( UInt32 f=0; f<run; ++f, sample+=channels )
            {
                const Float32 input = *sample;
                const Float32 fresh = RunSlot( m_live, c, input );
                if ( done + f < m_fadeLeft )
                {
                    const Float32 old = RunSlot( oldSlot, c, input );
                    *sample = old + weights[ f ] * ( fresh - old );
                }
                else
                    *sample = fresh;
            }
        }
        done += run;
    }

    if ( frames >= m_fadeLeft )
    {
        m_fading = false;
        m_fadeLeft = 0;
        m_configuration[ oldSlot ] = -1;
    }
    else
        m_fadeLeft -= frames;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
