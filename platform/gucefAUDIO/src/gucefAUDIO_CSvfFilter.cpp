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

#ifndef GUCEF_AUDIO_CBIQUADDESIGN_H
#include "gucefAUDIO_CBiquadDesign.h"
#define GUCEF_AUDIO_CBIQUADDESIGN_H
#endif /* GUCEF_AUDIO_CBIQUADDESIGN_H ? */

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#include "gucefAUDIO_CSvfFilter.h"
#define GUCEF_AUDIO_CSVFFILTER_H
#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */

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

static const Float64 Pi = 3.14159265358979323846;
static const Float64 MinQ = 0.001;
static const Float64 DenormalFlushThreshold = 1.0e-15;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSvfFilter::CSvfFilter( void )
    : m_a1( 1.0 )
    , m_a2( 0.0 )
    , m_a3( 0.0 )
    , m_m0( 1.0 )
    , m_m1( 0.0 )
    , m_m2( 0.0 )
    , m_state()
    , m_channelCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSvfFilter::~CSvfFilter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSvfFilter::Prepare( UInt32 channelCount )
{GUCEF_TRACE;

    m_state.assign( (size_t) channelCount * 2, 0.0 );
    m_channelCount = channelCount;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSvfFilter::GetChannelCount( void ) const
{
    return m_channelCount;
}

/*-------------------------------------------------------------------------*/

void
CSvfFilter::Reset( void )
{
    for ( size_t i=0; i<m_state.size(); ++i )
        m_state[ i ] = 0.0;
}

/*-------------------------------------------------------------------------*/

bool
CSvfFilter::SetResponse( TFilterResponse response ,
                         Float64 sampleRate       ,
                         Float64 frequency        ,
                         Float64 q                ,
                         Float64 gainInDecibels   )
{
    // no GUCEF_TRACE: may be called on the audio thread

    if ( !( sampleRate > 0.0 ) )
        return false;

    const Float64 maxFrequency = CBiquadDesign::GetMaxFrequency( sampleRate );
    if ( frequency > maxFrequency ) frequency = maxFrequency;
    if ( !( frequency > 0.0 ) ) frequency = 1.0e-3;
    if ( !( q > MinQ ) ) q = MinQ;

    Float64 g = tan( Pi * frequency / sampleRate );
    Float64 k = 1.0 / q;
    Float64 m0 = 0.0, m1 = 0.0, m2 = 0.0;

    switch ( response )
    {
        case FILTERRESPONSE_LOWPASS :
        {
            m2 = 1.0;
            break;
        }
        case FILTERRESPONSE_BANDPASS :
        {
            m1 = 1.0;
            break;
        }
        case FILTERRESPONSE_BANDPASS_PEAK0DB :
        {
            m1 = k;
            break;
        }
        case FILTERRESPONSE_HIGHPASS :
        {
            m0 = 1.0; m1 = -k; m2 = -1.0;
            break;
        }
        case FILTERRESPONSE_NOTCH :
        {
            m0 = 1.0; m1 = -k;
            break;
        }
        case FILTERRESPONSE_ALLPASS :
        {
            m0 = 1.0; m1 = -2.0 * k;
            break;
        }
        case FILTERRESPONSE_PEAKING :
        {
            const Float64 a = pow( 10.0, gainInDecibels / 40.0 );
            k = 1.0 / ( q * a );
            m0 = 1.0; m1 = k * ( a * a - 1.0 );
            break;
        }
        case FILTERRESPONSE_LOWSHELF :
        {
            const Float64 a = pow( 10.0, gainInDecibels / 40.0 );
            g /= sqrt( a );
            m0 = 1.0; m1 = k * ( a - 1.0 ); m2 = a * a - 1.0;
            break;
        }
        case FILTERRESPONSE_HIGHSHELF :
        {
            const Float64 a = pow( 10.0, gainInDecibels / 40.0 );
            g *= sqrt( a );
            m0 = a * a; m1 = k * ( 1.0 - a ) * a; m2 = 1.0 - a * a;
            break;
        }
        default :
        {
            m0 = 1.0;   // unknown response: pass through
            break;
        }
    }

    m_a1 = 1.0 / ( 1.0 + g * ( g + k ) );
    m_a2 = g * m_a1;
    m_a3 = g * m_a2;
    m_m0 = m0;
    m_m1 = m1;
    m_m2 = m2;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSvfFilter::ProcessInterleaved( const Float32* input, Float32* output, UInt32 frames )
{
    const UInt32 channels = m_channelCount;
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<channels; ++c )
            output[ (size_t) f * channels + c ] = Process( c, input[ (size_t) f * channels + c ] );
}

/*-------------------------------------------------------------------------*/

void
CSvfFilter::ProcessChannel( UInt32 channel, const Float32* input, Float32* output, UInt32 frames, UInt32 stride )
{
    // state in locals for the length of the loop
    Float64 ic1 = m_state[ (size_t) channel * 2 ];
    Float64 ic2 = m_state[ (size_t) channel * 2 + 1 ];
    const Float64 a1 = m_a1, a2 = m_a2, a3 = m_a3, m0 = m_m0, m1 = m_m1, m2 = m_m2;

    for ( UInt32 f=0; f<frames; ++f )
    {
        const Float64 v0 = (Float64) input[ (size_t) f * stride ];
        const Float64 v3 = v0 - ic2;
        const Float64 v1 = a1 * ic1 + a2 * v3;
        const Float64 v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0 * v1 - ic1;
        ic2 = 2.0 * v2 - ic2;
        output[ (size_t) f * stride ] = (Float32) ( m0 * v0 + m1 * v1 + m2 * v2 );
    }

    m_state[ (size_t) channel * 2 ] = ic1;
    m_state[ (size_t) channel * 2 + 1 ] = ic2;
}

/*-------------------------------------------------------------------------*/

void
CSvfFilter::FlushDenormals( void )
{
    for ( size_t i=0; i<m_state.size(); ++i )
        if ( fabs( m_state[ i ] ) < DenormalFlushThreshold )
            m_state[ i ] = 0.0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
