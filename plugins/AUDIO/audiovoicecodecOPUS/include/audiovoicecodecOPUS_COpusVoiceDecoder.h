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

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEDECODER_H
#define GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CVOICEDECODER_H
#include "gucefAUDIO_CVoiceDecoder.h"
#define GUCEF_AUDIO_CVOICEDECODER_H
#endif /* GUCEF_AUDIO_CVOICEDECODER_H ? */

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H
#include "audiovoicecodecOPUS_macros.h"
#define GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H
#endif /* GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H ? */

struct OpusDecoder;

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Opus decoder (see COpusVoiceCodecFactory): the concealment of a lost frame is the one of libopus, and the frame
 *  before a received one is recovered from the forward error correction data the received one carries, when the
 *  encoder had it on. The libopus state is allocated by Create() on the owner thread; the decoding calls allocate
 *  nothing, take no lock and do no IO.
 */
class COpusVoiceDecoder : public AUDIO::CVoiceDecoder
{
    public:

    /** Makes the libopus decoder. Null when libopus refuses the configuration */
    static COpusVoiceDecoder* Create( const AUDIO::CVoiceCodecConfig& config );

    virtual ~COpusVoiceDecoder() GUCEF_VIRTUAL_OVERRIDE;

    virtual const AUDIO::CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetOutputRate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** 48000: the RTP clock of Opus whatever the coding rate (RFC 7587 section 4.1) */
    virtual UInt32 GetTimestampRate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Conceal( Float32* pcm, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SupportsFec( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 DecodeFec( const UInt8* nextPayload, UInt32 size, Float32* pcm, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COpusVoiceDecoder( const AUDIO::CVoiceCodecConfig& config );

    COpusVoiceDecoder( const COpusVoiceDecoder& src );              /**< not implemented */
    COpusVoiceDecoder& operator=( const COpusVoiceDecoder& src );   /**< not implemented */

    private:

    AUDIO::CVoiceCodecConfig m_config;
    OpusDecoder* m_decoder;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OPUS */
}; /* namespace AUDIOVOICECODEC */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEDECODER_H ? */
