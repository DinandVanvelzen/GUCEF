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

#include "gucefAUDIO_CPanningSpatializer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CPanningSpatializer::TypeName = "panning";

/*-------------------------------------------------------------------------*/

const CString&
CPanningSpatializer::GetStaticSpatializerTypeName( void )
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

CPanningSpatializer::CPanningSpatializer( void )
    : IAudioSpatializer()
    , m_withCenter()
    , m_withoutCenter()
    , m_channelCount( 0 )
    , m_lfeChannel( -1 )
    , m_prepared( false )
    , m_gainsKnown( false )
{GUCEF_TRACE;

    for ( UInt32 c=0; c<CAudioPanner::MaxChannels; ++c )
        m_lastGains[ c ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

CPanningSpatializer::~CPanningSpatializer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CPanningSpatializer::GetSpatializerTypeName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

const CAudioPanner&
CPanningSpatializer::GetPanner( bool useCenterSpeaker ) const
{GUCEF_TRACE;

    return useCenterSpeaker ? m_withCenter : m_withoutCenter;
}

/*-------------------------------------------------------------------------*/

bool
CPanningSpatializer::Prepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_prepared = false;
    m_gainsKnown = false;
    if ( 0 == maxBlockFrames || !m_withCenter.Prepare( mixFormat, true ) || !m_withoutCenter.Prepare( mixFormat, false ) )
        return false;
    m_channelCount = m_withCenter.GetChannelCount();
    m_lfeChannel = m_withCenter.GetLfeChannel();
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPanningSpatializer::Reset( void ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // no trace macro: this runs where the audio is mixed
    m_gainsKnown = false;
}

/*-------------------------------------------------------------------------*/

void
CPanningSpatializer::Render( const Float32* mono, UInt32 frames, const SSpatialFrame& frame, Float32* output ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // no trace macro: this runs where the audio is mixed
    if ( !m_prepared || 0 == frames )
        return;

    const UInt32 channels = m_channelCount;
    Float32 gains[ CAudioPanner::MaxChannels ];
    const CAudioPanner& panner = frame.useCenterSpeaker ? m_withCenter : m_withoutCenter;
    panner.GetGains( frame.azimuth, frame.pan, frame.spreadFactor, frame.rearGain, gains );
    if ( m_lfeChannel >= 0 )
        gains[ m_lfeChannel ] = frame.lfeGain > 0.0f ? frame.lfeGain : 0.0f;
    for ( UInt32 c=0; c<channels; ++c )
        gains[ c ] *= frame.gain;

    if ( !m_gainsKnown )
    {
        for ( UInt32 c=0; c<channels; ++c )
            m_lastGains[ c ] = gains[ c ];
        m_gainsKnown = true;
    }

    // a ramp from the gains of the previous block to these, per channel; silent channels are left alone
    const Float32 inverseFrames = 1.0f / (Float32) frames;
    for ( UInt32 c=0; c<channels; ++c )
    {
        const Float32 from = m_lastGains[ c ];
        const Float32 to = gains[ c ];
        m_lastGains[ c ] = to;
        if ( 0.0f == from && 0.0f == to )
            continue;

        Float32* into = output + c;
        if ( from == to )
        {
            for ( UInt32 f=0; f<frames; ++f )
                into[ (size_t) f * channels ] += mono[ f ] * to;
        }
        else
        {
            const Float32 step = to - from;
            for ( UInt32 f=0; f<frames; ++f )
                into[ (size_t) f * channels ] += mono[ f ] * ( from + step * ( (Float32) ( f + 1 ) * inverseFrames ) );
        }
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
