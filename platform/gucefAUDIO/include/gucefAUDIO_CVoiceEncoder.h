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

#ifndef GUCEF_AUDIO_CVOICEENCODER_H
#define GUCEF_AUDIO_CVOICEENCODER_H

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
 *  Encodes mono audio in frames of a fixed duration. Made by a CVoiceCodecFactory and prepared on the owner thread.
 *
 *  Real time contract: Encode(), Reset() and the getters allocate no memory, take no lock, do no IO and take a bounded
 *  time for one frame, so the capture thread can call them. SetBitrate() and SetExpectedLossPercentage() may be called
 *  from any thread, the codec applies the value at the next Encode(). An encoder is used by one thread at a time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceEncoder
{
    public:

    typedef CORE::CTSharedPtr< CVoiceEncoder, MT::CMutex > TVoiceEncoderPtr;

    CVoiceEncoder( void );

    virtual ~CVoiceEncoder();

    /** The configuration this encoder was made with */
    virtual const CVoiceCodecConfig& GetConfig( void ) const = 0;

    /** Samples in one frame at the codec's sample rate */
    UInt32 GetFrameSizeInFrames( void ) const;

    /** The largest payload Encode() can produce, in bytes */
    virtual UInt32 GetMaxPayloadSize( void ) const = 0;

    /** Delay of the codec in samples at the codec's sample rate (the decoded audio is this far behind the input) */
    virtual UInt32 GetLookaheadInFrames( void ) const = 0;

    /** The clock rate of the RTP timestamps: 48000 for Opus (RFC 7587), the sample rate for PCM */
    virtual UInt32 GetTimestampRate( void ) const = 0;

    /**
     *  Encodes exactly one frame (GetFrameSizeInFrames() samples, full scale is 1.0) into 'payload'.
     *  Real time safe. Returns the bytes written, 0 when there is nothing to send for this frame, < 0 on error
     *  (a capacity that is too small, for example)
     */
    virtual Int32 Encode( const Float32* frame, UInt8* payload, UInt32 capacity ) = 0;

    /** Any thread: the target bit rate, applied at the next Encode(). Codecs without a bit rate ignore it */
    virtual void SetBitrate( UInt32 bitsPerSecond );

    /** Any thread: the packet loss in percent the encoder prepares for, applied at the next Encode() */
    virtual void SetExpectedLossPercentage( UInt32 percentage );

    /** Forgets the history of the signal, as at the start of a stream. Real time safe */
    virtual void Reset( void ) = 0;

    private:
    friend class CVoiceCodecFactory;

    CVoiceEncoder( const CVoiceEncoder& src );              /**< not implemented */
    CVoiceEncoder& operator=( const CVoiceEncoder& src );   /**< not implemented */

    private:

    CVoiceCodecFactory* m_factory;
};

typedef CVoiceEncoder::TVoiceEncoderPtr TVoiceEncoderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEENCODER_H ? */
