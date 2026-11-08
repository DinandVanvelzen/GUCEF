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

#ifndef GUCEF_AUDIO_CVOICEDECODER_H
#define GUCEF_AUDIO_CVOICEDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CVOICECODECCONFIG_H
#include "gucefAUDIO_CVoiceCodecConfig.h"
#define GUCEF_AUDIO_CVOICECODECCONFIG_H
#endif /* GUCEF_AUDIO_CVOICECODECCONFIG_H ? */

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

class CVoiceCodecFactory;

/*-------------------------------------------------------------------------*/

/**
 *  Decodes frames made by a CVoiceEncoder, with concealment of a lost frame and, for codecs that have it, recovery
 *  of a lost frame from the next one (forward error correction). Made by a CVoiceCodecFactory and prepared on the
 *  owner thread.
 *
 *  Real time contract: Decode(), Conceal(), DecodeFec(), Reset() and the getters allocate no memory, take no lock,
 *  do no IO and take a bounded time for one frame, so the render thread can call them. A decoder is used by one
 *  thread at a time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceDecoder
{
    public:

    typedef CORE::CTSharedPtr< CVoiceDecoder, MT::CMutex > TVoiceDecoderPtr;

    CVoiceDecoder( void );

    virtual ~CVoiceDecoder();

    /** The configuration this decoder was made with */
    virtual const CVoiceCodecConfig& GetConfig( void ) const = 0;

    /** Samples per second of the audio the decoder makes */
    virtual UInt32 GetOutputRate( void ) const = 0;

    /**
     *  The clock rate of the RTP timestamps of the frames this decoder takes: 48000 for Opus (RFC 7587), the sample rate
     *  for PCM. The default is the sample rate of the configuration
     */
    virtual UInt32 GetTimestampRate( void ) const;

    /**
     *  Decodes one frame into 'pcm'. Real time safe. Returns the number of samples written (< 0 on error: a payload
     *  that is not a frame of this codec, or a capacity that is too small)
     */
    virtual Int32 Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity ) = 0;

    /** Makes the audio of one lost frame of 'frames' samples. Real time safe. Returns the samples written (< 0 on error) */
    virtual Int32 Conceal( Float32* pcm, UInt32 frames ) = 0;

    /** True when DecodeFec() works (default false) */
    virtual bool SupportsFec( void ) const;

    /**
     *  Recovers the frame that came before the frame in 'nextPayload' from the data the next frame carries.
     *  Real time safe. Returns the samples written (< 0 when not supported or on error)
     */
    virtual Int32 DecodeFec( const UInt8* nextPayload, UInt32 size, Float32* pcm, UInt32 frames );

    /** Forgets the history of the signal, as at the start of a stream. Real time safe */
    virtual void Reset( void ) = 0;

    private:
    friend class CVoiceCodecFactory;

    CVoiceDecoder( const CVoiceDecoder& src );              /**< not implemented */
    CVoiceDecoder& operator=( const CVoiceDecoder& src );   /**< not implemented */

    private:

    CVoiceCodecFactory* m_factory;
};

typedef CVoiceDecoder::TVoiceDecoderPtr TVoiceDecoderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEDECODER_H ? */
