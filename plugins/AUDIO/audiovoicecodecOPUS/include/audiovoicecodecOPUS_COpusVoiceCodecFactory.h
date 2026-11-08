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

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICECODECFACTORY_H
#define GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICECODECFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CVOICECODECFACTORY_H
#include "gucefAUDIO_CVoiceCodecFactory.h"
#define GUCEF_AUDIO_CVOICECODECFACTORY_H
#endif /* GUCEF_AUDIO_CVOICECODECFACTORY_H ? */

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H
#include "audiovoicecodecOPUS_macros.h"
#define GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H
#endif /* GUCEF_AUDIOVOICECODEC_OPUS_MACROS_H ? */

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
 *  Factory of the "opus" voice codec (libopus). Supported: one channel at 8, 12, 16, 24 or 48 kHz, and the two profiles
 *
 *   - lowDelay: OPUS_APPLICATION_RESTRICTED_LOWDELAY (CELT only), frames of 2.5, 5, 10 or 20 ms, no forward error
 *     correction (CELT has none: the setting is ignored), delay of the frame plus 2.5 ms
 *   - robust: OPUS_APPLICATION_VOIP with forward error correction on when asked, frames of 20 ms
 *
 *  Opus DTX is not used: silence is handled by the transmit gating of the sender.
 */
class COpusVoiceCodecFactory : public AUDIO::CVoiceCodecFactory
{
    public:

    COpusVoiceCodecFactory( void );

    virtual ~COpusVoiceCodecFactory() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetCodecTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSupported( const AUDIO::CVoiceCodecConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual AUDIO::CVoiceEncoder* OnCreateEncoder( const AUDIO::CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual AUDIO::CVoiceDecoder* OnCreateDecoder( const AUDIO::CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COpusVoiceCodecFactory( const COpusVoiceCodecFactory& src );                /**< not implemented */
    COpusVoiceCodecFactory& operator=( const COpusVoiceCodecFactory& src );     /**< not implemented */
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

#endif /* GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICECODECFACTORY_H ? */
