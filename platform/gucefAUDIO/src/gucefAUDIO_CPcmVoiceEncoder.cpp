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

#include "gucefAUDIO_CPcmVoiceEncoder.h"

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

CPcmVoiceEncoder::CPcmVoiceEncoder( const CVoiceCodecConfig& config )
    : CVoiceEncoder()
    , m_config( config )
    , m_frameSize( config.GetFrameSizeInFrames() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPcmVoiceEncoder::~CPcmVoiceEncoder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CVoiceCodecConfig&
CPcmVoiceEncoder::GetConfig( void ) const
{
    return m_config;
}

/*-------------------------------------------------------------------------*/

UInt32
CPcmVoiceEncoder::GetMaxPayloadSize( void ) const
{
    return m_frameSize * 2;
}

/*-------------------------------------------------------------------------*/

UInt32
CPcmVoiceEncoder::GetLookaheadInFrames( void ) const
{
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CPcmVoiceEncoder::GetTimestampRate( void ) const
{
    return m_config.GetSampleRate();
}

/*-------------------------------------------------------------------------*/

Int32
CPcmVoiceEncoder::Encode( const Float32* frame, UInt8* payload, UInt32 capacity )
{
    const UInt32 needed = m_frameSize * 2;
    if ( capacity < needed )
        return -1;

    for ( UInt32 i=0; i<m_frameSize; ++i )
    {
        // full scale is 32768: rounds half away from zero and saturates, and a NaN is silence
        const Float32 scaled = frame[ i ] * 32768.0f;
        Int32 value = 0;
        if ( scaled >= 32767.0f )
            value = 32767;
        else
        if ( scaled <= -32768.0f )
            value = -32768;
        else
        if ( scaled >= 0.0f )
            value = (Int32) ( scaled + 0.5f );
        else
        if ( scaled < 0.0f )
            value = -(Int32) ( 0.5f - scaled );

        const UInt32 bits = (UInt32) ( value & 0xFFFF );
        payload[ i * 2 ] = (UInt8) ( bits >> 8 );
        payload[ i * 2 + 1 ] = (UInt8) ( bits & 0xFF );
    }
    return (Int32) needed;
}

/*-------------------------------------------------------------------------*/

void
CPcmVoiceEncoder::Reset( void )
{
    // no state
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
