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

#include "audiovoicecodecOPUS_COpusVoiceEncoder.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The largest Opus packet of one frame */
static const UInt32 MaxOpusPacketSize = 1276;

static UInt32
ClampBitrate( UInt32 bitsPerSecond )
{
    if ( bitsPerSecond < 500 )      return 500;
    if ( bitsPerSecond > 512000 )   return 512000;
    return bitsPerSecond;
}

static UInt32
ClampLoss( UInt32 percentage )
{
    return percentage > 100 ? 100 : percentage;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COpusVoiceEncoder::COpusVoiceEncoder( const AUDIO::CVoiceCodecConfig& config )
    : AUDIO::CVoiceEncoder()
    , m_config( config )
    , m_encoder( GUCEF_NULL )
    , m_frameSize( config.GetFrameSizeInFrames() )
    , m_lookahead( 0 )
    , m_appliedBitrate( ClampBitrate( config.GetBitrate() ) )
    , m_appliedLoss( ClampLoss( config.GetExpectedLossPercentage() ) )
    , m_publishedBitrate( ClampBitrate( config.GetBitrate() ) )
    , m_publishedLoss( ClampLoss( config.GetExpectedLossPercentage() ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpusVoiceEncoder::~COpusVoiceEncoder()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_encoder )
        opus_encoder_destroy( m_encoder );
    m_encoder = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

COpusVoiceEncoder*
COpusVoiceEncoder::Create( const AUDIO::CVoiceCodecConfig& config )
{GUCEF_TRACE;

    const bool robust = AUDIO::VOICECODECPROFILE_ROBUST == config.GetProfile();

    COpusVoiceEncoder* encoder = GUCEF_NEW COpusVoiceEncoder( config );

    int error = OPUS_OK;
    encoder->m_encoder = opus_encoder_create( (opus_int32) config.GetSampleRate(), 1, robust ? OPUS_APPLICATION_VOIP : OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error );
    if ( OPUS_OK != error || GUCEF_NULL == encoder->m_encoder )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OpusVoiceEncoder: opus_encoder_create failed with error " << error << ": " << opus_strerror( error ) );
        encoder->m_encoder = GUCEF_NULL;
        GUCEF_DELETE encoder;
        return GUCEF_NULL;
    }

    OpusEncoder* opus = encoder->m_encoder;
    const UInt32 complexity = config.GetComplexity() > 10 ? 10 : config.GetComplexity();
    int result = opus_encoder_ctl( opus, OPUS_SET_SIGNAL( OPUS_SIGNAL_VOICE ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_VBR( 1 ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_BITRATE( (opus_int32) encoder->m_appliedBitrate ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_COMPLEXITY( (opus_int32) complexity ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_DTX( 0 ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_PACKET_LOSS_PERC( (opus_int32) encoder->m_appliedLoss ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_INBAND_FEC( robust && config.GetInbandFec() ? 1 : 0 ) );
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_SET_FORCE_CHANNELS( 1 ) );

    opus_int32 lookahead = 0;
    result = result < 0 ? result : opus_encoder_ctl( opus, OPUS_GET_LOOKAHEAD( &lookahead ) );
    if ( result < 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OpusVoiceEncoder: configuring libopus failed with error " << result << ": " << opus_strerror( result ) );
        GUCEF_DELETE encoder;
        return GUCEF_NULL;
    }
    encoder->m_lookahead = lookahead > 0 ? (UInt32) lookahead : 0;
    return encoder;
}

/*-------------------------------------------------------------------------*/

const AUDIO::CVoiceCodecConfig&
COpusVoiceEncoder::GetConfig( void ) const
{
    return m_config;
}

/*-------------------------------------------------------------------------*/

UInt32
COpusVoiceEncoder::GetMaxPayloadSize( void ) const
{
    return MaxOpusPacketSize;
}

/*-------------------------------------------------------------------------*/

UInt32
COpusVoiceEncoder::GetLookaheadInFrames( void ) const
{
    return m_lookahead;
}

/*-------------------------------------------------------------------------*/

UInt32
COpusVoiceEncoder::GetTimestampRate( void ) const
{
    // RFC 7587 section 4.1: the RTP clock of Opus is 48 kHz whatever the coding rate
    return 48000;
}

/*-------------------------------------------------------------------------*/

Int32
COpusVoiceEncoder::Encode( const Float32* frame, UInt8* payload, UInt32 capacity )
{
    if ( capacity < 2 )
        return -1;

    // the settings published by other threads, applied here: control calls that allocate nothing
    const UInt32 bitrate = m_publishedBitrate.Load( MT::MEMORDER_ACQUIRE );
    if ( bitrate != m_appliedBitrate )
    {
        if ( opus_encoder_ctl( m_encoder, OPUS_SET_BITRATE( (opus_int32) bitrate ) ) >= 0 )
            m_appliedBitrate = bitrate;
    }
    const UInt32 loss = m_publishedLoss.Load( MT::MEMORDER_ACQUIRE );
    if ( loss != m_appliedLoss )
    {
        if ( opus_encoder_ctl( m_encoder, OPUS_SET_PACKET_LOSS_PERC( (opus_int32) loss ) ) >= 0 )
            m_appliedLoss = loss;
    }

    const UInt32 limit = capacity < MaxOpusPacketSize ? capacity : MaxOpusPacketSize;
    return (Int32) opus_encode_float( m_encoder, frame, (int) m_frameSize, payload, (opus_int32) limit );
}

/*-------------------------------------------------------------------------*/

void
COpusVoiceEncoder::SetBitrate( UInt32 bitsPerSecond )
{
    m_publishedBitrate.Store( ClampBitrate( bitsPerSecond ), MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

void
COpusVoiceEncoder::SetExpectedLossPercentage( UInt32 percentage )
{
    m_publishedLoss.Store( ClampLoss( percentage ), MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

void
COpusVoiceEncoder::Reset( void )
{
    opus_encoder_ctl( m_encoder, OPUS_RESET_STATE );
    // a reset restores the defaults of libopus for the state it clears, the settings are kept by it; applied values stay valid
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
