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

#ifndef GUCEF_AUDIO_CPCMVOICEDECODER_H
#define GUCEF_AUDIO_CPCMVOICEDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CVOICEDECODER_H
#include "gucefAUDIO_CVoiceDecoder.h"
#define GUCEF_AUDIO_CVOICEDECODER_H
#endif /* GUCEF_AUDIO_CVOICEDECODER_H ? */

#ifndef GUCEF_AUDIO_CPACKETLOSSCONCEALER_H
#include "gucefAUDIO_CPacketLossConcealer.h"
#define GUCEF_AUDIO_CPACKETLOSSCONCEALER_H
#endif /* GUCEF_AUDIO_CPACKETLOSSCONCEALER_H ? */

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
 *  Decoder of the built-in "pcm" voice codec (16 bit linear PCM in network byte order, RFC 3551 L16). The
 *  decoded frames are bit-exact what the encoder was given, to the resolution of 16 bits. A lost frame is made up by
 *  a CPacketLossConcealer; PCM has no forward error correction.
 */
class GUCEF_AUDIO_PUBLIC_CPP CPcmVoiceDecoder : public CVoiceDecoder
{
    public:

    CPcmVoiceDecoder( const CVoiceCodecConfig& config );

    virtual ~CPcmVoiceDecoder() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetOutputRate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Conceal( Float32* pcm, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The concealer, for tests and statistics */
    const CPacketLossConcealer& GetConcealer( void ) const;

    private:

    CPcmVoiceDecoder( const CPcmVoiceDecoder& src );                /**< not implemented */
    CPcmVoiceDecoder& operator=( const CPcmVoiceDecoder& src );     /**< not implemented */

    private:

    CVoiceCodecConfig m_config;
    CPacketLossConcealer m_concealer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CPCMVOICEDECODER_H ? */
