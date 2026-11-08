/*
 *  audiovoicecodecOPUS: gucefAUDIO voice codec plugin for Opus
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

#include "opus.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "audiovoicecodecOPUS_COpusVoiceDecoder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIOVOICECODEC {
namespace OPUS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COpusVoiceDecoder::COpusVoiceDecoder( const AUDIO::CVoiceCodecConfig& config )
    : AUDIO::CVoiceDecoder()
    , m_config( config )
    , m_decoder( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpusVoiceDecoder::~COpusVoiceDecoder()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_decoder )
        opus_decoder_destroy( m_decoder );
    m_decoder = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

COpusVoiceDecoder*
COpusVoiceDecoder::Create( const AUDIO::CVoiceCodecConfig& config )
{GUCEF_TRACE;

    COpusVoiceDecoder* decoder = GUCEF_NEW COpusVoiceDecoder( config );

    int error = OPUS_OK;
    decoder->m_decoder = opus_decoder_create( (opus_int32) config.GetSampleRate(), 1, &error );
    if ( OPUS_OK != error || GUCEF_NULL == decoder->m_decoder )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OpusVoiceDecoder: opus_decoder_create failed with error " << error << ": " << opus_strerror( error ) );
        decoder->m_decoder = GUCEF_NULL;
        GUCEF_DELETE decoder;
        return GUCEF_NULL;
    }
    return decoder;
}

/*-------------------------------------------------------------------------*/

const AUDIO::CVoiceCodecConfig&
COpusVoiceDecoder::GetConfig( void ) const
{
    return m_config;
}

/*-------------------------------------------------------------------------*/

UInt32
COpusVoiceDecoder::GetOutputRate( void ) const
{
    return m_config.GetSampleRate();
}

/*-------------------------------------------------------------------------*/

UInt32
COpusVoiceDecoder::GetTimestampRate( void ) const
{
    return 48000;
}

/*-------------------------------------------------------------------------*/

Int32
COpusVoiceDecoder::Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity )
{
    // a packet of no bytes is how libopus is told about a loss: that is Conceal(), not a frame
    if ( 0 == size )
        return -1;
    return (Int32) opus_decode_float( m_decoder, payload, (opus_int32) size, pcm, (int) frameCapacity, 0 );
}

/*-------------------------------------------------------------------------*/

Int32
COpusVoiceDecoder::Conceal( Float32* pcm, UInt32 frames )
{
    return (Int32) opus_decode_float( m_decoder, GUCEF_NULL, 0, pcm, (int) frames, 0 );
}

/*-------------------------------------------------------------------------*/

bool
COpusVoiceDecoder::SupportsFec( void ) const
{
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
COpusVoiceDecoder::DecodeFec( const UInt8* nextPayload, UInt32 size, Float32* pcm, UInt32 frames )
{
    if ( 0 == size )
        return -1;
    return (Int32) opus_decode_float( m_decoder, nextPayload, (opus_int32) size, pcm, (int) frames, 1 );
}

/*-------------------------------------------------------------------------*/

void
COpusVoiceDecoder::Reset( void )
{
    opus_decoder_ctl( m_decoder, OPUS_RESET_STATE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OPUS */
}; /* namespace AUDIOVOICECODEC */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
