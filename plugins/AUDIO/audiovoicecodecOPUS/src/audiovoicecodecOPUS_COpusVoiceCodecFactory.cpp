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

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEENCODER_H
#include "audiovoicecodecOPUS_COpusVoiceEncoder.h"
#define GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEENCODER_H
#endif /* GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEENCODER_H ? */

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEDECODER_H
#include "audiovoicecodecOPUS_COpusVoiceDecoder.h"
#define GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEDECODER_H
#endif /* GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICEDECODER_H ? */

#include "audiovoicecodecOPUS_COpusVoiceCodecFactory.h"

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

COpusVoiceCodecFactory::COpusVoiceCodecFactory( void )
    : AUDIO::CVoiceCodecFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpusVoiceCodecFactory::~COpusVoiceCodecFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COpusVoiceCodecFactory::GetCodecTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString typeName = "opus";
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
COpusVoiceCodecFactory::IsSupported( const AUDIO::CVoiceCodecConfig& config ) const
{GUCEF_TRACE;

    if ( 1 != config.GetChannels() )
        return false;

    const UInt32 rate = config.GetSampleRate();
    if ( 8000 != rate && 12000 != rate && 16000 != rate && 24000 != rate && 48000 != rate )
        return false;

    const UInt32 duration = config.GetFrameDurationInUs();
    if ( AUDIO::VOICECODECPROFILE_ROBUST == config.GetProfile() )
        return 20000 == duration;
    return 2500 == duration || 5000 == duration || 10000 == duration || 20000 == duration;
}

/*-------------------------------------------------------------------------*/

AUDIO::CVoiceEncoder*
COpusVoiceCodecFactory::OnCreateEncoder( const AUDIO::CVoiceCodecConfig& config )
{GUCEF_TRACE;

    return COpusVoiceEncoder::Create( config );
}

/*-------------------------------------------------------------------------*/

AUDIO::CVoiceDecoder*
COpusVoiceCodecFactory::OnCreateDecoder( const AUDIO::CVoiceCodecConfig& config )
{GUCEF_TRACE;

    return COpusVoiceDecoder::Create( config );
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
