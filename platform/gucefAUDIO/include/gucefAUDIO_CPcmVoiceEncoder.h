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

#ifndef GUCEF_AUDIO_CPCMVOICEENCODER_H
#define GUCEF_AUDIO_CPCMVOICEENCODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CVOICEENCODER_H
#include "gucefAUDIO_CVoiceEncoder.h"
#define GUCEF_AUDIO_CVOICEENCODER_H
#endif /* GUCEF_AUDIO_CVOICEENCODER_H ? */

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
 *  Encoder of the built-in "pcm" voice codec: 16 bit linear PCM, signed, in network byte order (RFC 3551 L16), with
 *  rounding and saturation. No delay, no state: the frames are independent.
 */
class GUCEF_AUDIO_PUBLIC_CPP CPcmVoiceEncoder : public CVoiceEncoder
{
    public:

    CPcmVoiceEncoder( const CVoiceCodecConfig& config );

    virtual ~CPcmVoiceEncoder() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetMaxPayloadSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetLookaheadInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetTimestampRate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Encode( const Float32* frame, UInt8* payload, UInt32 capacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPcmVoiceEncoder( const CPcmVoiceEncoder& src );                /**< not implemented */
    CPcmVoiceEncoder& operator=( const CPcmVoiceEncoder& src );     /**< not implemented */

    private:

    CVoiceCodecConfig m_config;
    UInt32 m_frameSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CPCMVOICEENCODER_H ? */
