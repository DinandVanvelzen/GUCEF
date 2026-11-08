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

#include "gucefAUDIO_CPcmVoiceDecoder.h"

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

CPcmVoiceDecoder::CPcmVoiceDecoder( const CVoiceCodecConfig& config )
    : CVoiceDecoder()
    , m_config( config )
    , m_concealer()
{GUCEF_TRACE;

    m_concealer.Prepare( config.GetSampleRate() );
}

/*-------------------------------------------------------------------------*/

CPcmVoiceDecoder::~CPcmVoiceDecoder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CVoiceCodecConfig&
CPcmVoiceDecoder::GetConfig( void ) const
{
    return m_config;
}

/*-------------------------------------------------------------------------*/

UInt32
CPcmVoiceDecoder::GetOutputRate( void ) const
{
    return m_config.GetSampleRate();
}

/*-------------------------------------------------------------------------*/

Int32
CPcmVoiceDecoder::Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity )
{
    if ( 0 == size || 0 != ( size & 1 ) )
        return -1;
    const UInt32 frames = size / 2;
    if ( frames > frameCapacity )
        return -1;

    for ( UInt32 i=0; i<frames; ++i )
    {
        Int32 value = (Int32) ( ( (UInt32) payload[ i * 2 ] << 8 ) | (UInt32) payload[ i * 2 + 1 ] );
        if ( value >= 32768 )
            value -= 65536;
        pcm[ i ] = (Float32) value * ( 1.0f / 32768.0f );
    }
    m_concealer.OnGoodFrame( pcm, frames );
    return (Int32) frames;
}

/*-------------------------------------------------------------------------*/

Int32
CPcmVoiceDecoder::Conceal( Float32* pcm, UInt32 frames )
{
    m_concealer.Conceal( pcm, frames );
    return (Int32) frames;
}

/*-------------------------------------------------------------------------*/

void
CPcmVoiceDecoder::Reset( void )
{
    m_concealer.Reset();
}

/*-------------------------------------------------------------------------*/

const CPacketLossConcealer&
CPcmVoiceDecoder::GetConcealer( void ) const
{GUCEF_TRACE;

    return m_concealer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
