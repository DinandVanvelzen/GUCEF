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

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEENCODER_H
#define GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEENCODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_CVOICEENCODER_H
#include "gucefAUDIO_CVoiceEncoder.h"
#define GUCEF_AUDIO_CVOICEENCODER_H
#endif /* GUCEF_AUDIO_CVOICEENCODER_H ? */

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H
#include "audiovoicecodecOPUS_macros.h"
#define GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H
#endif /* GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H ? */

struct OpusEncoder;

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
 *  Opus encoder of one profile (see COpusVoiceCodecFactory). The libopus state is allocated by Create() on the owner
 *  thread; Encode() allocates nothing (libopus is built with USE_ALLOCA, so its scratch memory is on the stack),
 *  takes no lock and does no IO. The bit rate and the expected loss are published with atomics by any thread and applied by
 *  control calls at the next Encode().
 */
class COpusVoiceEncoder : public AUDIO::CVoiceEncoder
{
    public:

    /** Makes and configures the libopus encoder. Null when libopus refuses the configuration */
    static COpusVoiceEncoder* Create( const AUDIO::CVoiceCodecConfig& config );

    virtual ~COpusVoiceEncoder() GUCEF_VIRTUAL_OVERRIDE;

    virtual const AUDIO::CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetMaxPayloadSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetLookaheadInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetTimestampRate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Encode( const Float32* frame, UInt8* payload, UInt32 capacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetBitrate( UInt32 bitsPerSecond ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetExpectedLossPercentage( UInt32 percentage ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COpusVoiceEncoder( const AUDIO::CVoiceCodecConfig& config );

    COpusVoiceEncoder( const COpusVoiceEncoder& src );              /**< not implemented */
    COpusVoiceEncoder& operator=( const COpusVoiceEncoder& src );   /**< not implemented */

    private:

    AUDIO::CVoiceCodecConfig m_config;
    OpusEncoder* m_encoder;
    UInt32 m_frameSize;
    UInt32 m_lookahead;
    UInt32 m_appliedBitrate;                        /**< owned by the encoding thread */
    UInt32 m_appliedLoss;
    MT::CTAtomic< UInt32 > m_publishedBitrate;
    MT::CTAtomic< UInt32 > m_publishedLoss;
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

#endif /* GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEENCODER_H ? */
