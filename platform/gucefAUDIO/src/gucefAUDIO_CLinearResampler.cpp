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

#include "gucefAUDIO_CLinearResampler.h"

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

CLinearResampler::CLinearResampler( void )
    : IAudioResampler()
    , m_channelCount( 0 )
    , m_step( 1.0 )
    , m_position( 0.0 )
    , m_previous()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinearResampler::~CLinearResampler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLinearResampler::Configure( UInt32 channelCount, Float64 inputRate, Float64 outputRate )
{GUCEF_TRACE;

    if ( 0 == channelCount || inputRate <= 0.0 || outputRate <= 0.0 )
        return false;

    if ( channelCount != m_channelCount )
    {
        m_previous.assign( channelCount, 0.0f );
        m_channelCount = channelCount;
        Reset();
    }
    return SetRates( inputRate, outputRate );
}

/*-------------------------------------------------------------------------*/

bool
CLinearResampler::SetRates( Float64 inputRate, Float64 outputRate )
{
    // no trace macro: this runs where the audio is mixed

    if ( inputRate <= 0.0 || outputRate <= 0.0 )
        return false;
    m_step = inputRate / outputRate;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLinearResampler::Reset( void )
{
    // no trace macro: this runs where the audio is mixed

    m_position = 0.0;
    for ( size_t c=0; c<m_previous.size(); ++c )
        m_previous[ c ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CLinearResampler::Process( const Float32* input          ,
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

    // Frame -1 is the last frame of the previous chunk, 0 .. inputFrames-1 are the new ones.
    // An output at position p lies between frame floor(p) and floor(p)+1, so it can only be made while that
    // next frame exists. The outputs that need frames that have not arrived yet wait for the next chunk.
    const UInt32 channels = m_channelCount;
    const Float64 lastUsable = (Float64) inputFrames - 1.0;
    Float64 position = m_position;
    UInt32 produced = 0;

    while ( produced < outputFrameCapacity && position < lastUsable )
    {
        const Int64 index = (Int64) floor( position );
        const Float32 fraction = (Float32) ( position - (Float64) index );
        for ( UInt32 c=0; c<channels; ++c )
        {
            const Float32 a = ( index < 0 ) ? m_previous[ c ] : input[ (size_t) index * channels + c ];
            const Float32 b = input[ (size_t) ( index + 1 ) * channels + c ];
            output[ (size_t) produced * channels + c ] = a + ( b - a ) * fraction;
        }
        ++produced;
        position += m_step;
    }

    if ( position >= lastUsable )
    {
        // Everything of this chunk is used up: remember its last frame and shift the position into the next chunk
        for ( UInt32 c=0; c<channels; ++c )
            m_previous[ c ] = input[ (size_t) ( inputFrames - 1 ) * channels + c ];
        m_position = position - (Float64) inputFrames;
        inputFramesConsumed = inputFrames;
    }
    else
    {
        // The output space ran out first. Offer the same input again with room for more.
        // The position stays relative to the start of this chunk, no input is consumed.
        m_position = position;
        inputFramesConsumed = 0;
    }
    outputFramesProduced = produced;
}

/*-------------------------------------------------------------------------*/

UInt32
CLinearResampler::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

IAudioResampler*
CLinearResampler::Clone( void ) const
{GUCEF_TRACE;

    CLinearResampler* clone = GUCEF_NEW CLinearResampler();
    if ( m_channelCount > 0 )
        clone->Configure( m_channelCount, m_step, 1.0 );
    return clone;
}

/*-------------------------------------------------------------------------*/

const char*
CLinearResampler::GetName( void ) const
{GUCEF_TRACE;

    return "linear";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
