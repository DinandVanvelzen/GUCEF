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

#ifndef GUCEF_AUDIO_CPCMVOICECODECFACTORY_H
#define GUCEF_AUDIO_CPCMVOICECODECFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CVOICECODECFACTORY_H
#include "gucefAUDIO_CVoiceCodecFactory.h"
#define GUCEF_AUDIO_CVOICECODECFACTORY_H
#endif /* GUCEF_AUDIO_CVOICECODECFACTORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Factory of the built-in "pcm" voice codec: 16 bit linear PCM in network byte order (RFC 3551 L16). It needs no
 *  dependency and is bit-exact, which makes it the reference for tests; it costs 768 kbit/s at 48 kHz or 256 kbit/s at
 *  16 kHz, acceptable on wired Ethernet.
 *
 *  Supported: one channel, sample rates from 8000 to 48000 Hz, frames of 2.5, 5, 10 or 20 ms that are a whole number
 *  of samples and whose payload fits CVoiceFrame::MaxPayloadSize (1440 bytes): at 48 kHz that is up to 10 ms of the
 *  durations offered (15 ms would fit), at 16 kHz up to 20 ms.
 */
class GUCEF_AUDIO_PUBLIC_CPP CPcmVoiceCodecFactory : public CVoiceCodecFactory
{
    public:

    CPcmVoiceCodecFactory( void );

    virtual ~CPcmVoiceCodecFactory() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetCodecTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSupported( const CVoiceCodecConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual CVoiceEncoder* OnCreateEncoder( const CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CVoiceDecoder* OnCreateDecoder( const CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPcmVoiceCodecFactory( const CPcmVoiceCodecFactory& src );              /**< not implemented */
    CPcmVoiceCodecFactory& operator=( const CPcmVoiceCodecFactory& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CPCMVOICECODECFACTORY_H ? */
