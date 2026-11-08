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

#ifndef GUCEF_AUDIO_CVOICECODECFACTORY_H
#define GUCEF_AUDIO_CVOICECODECFACTORY_H

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

#ifndef GUCEF_AUDIO_CVOICEDECODER_H
#include "gucefAUDIO_CVoiceDecoder.h"
#define GUCEF_AUDIO_CVOICEDECODER_H
#endif /* GUCEF_AUDIO_CVOICEDECODER_H ? */

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
 *  Makes the encoders and decoders of one voice codec. Registered in CVoiceCodecRegistry under its codec type name, by
 *  CAudioGlobal for the built-in codecs and by voice codec plugins (C++ link-back plugins, as audio drivers are) for theirs.
 *
 *  A factory counts the encoders and decoders it made that are still alive. A plugin must not be unloaded while any of
 *  them live, because their code is in the plugin: CVoiceCodecRegistry::TryUnregisterFactory() refuses in that case, and
 *  the factory must outlive every encoder and decoder it made.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceCodecFactory
{
    public:

    typedef CORE::CTSharedPtr< CVoiceCodecFactory, MT::CMutex > TVoiceCodecFactoryPtr;

    CVoiceCodecFactory( void );

    virtual ~CVoiceCodecFactory();

    /** Type name of the codec this factory makes encoders and decoders of, the key in the registry */
    virtual const CString& GetCodecTypeName( void ) const = 0;

    /** Whether the codec can work with the configuration (sample rate, channels, frame duration, settings) */
    virtual bool IsSupported( const CVoiceCodecConfig& config ) const = 0;

    /** Makes an encoder, prepared on this thread. False, and 'encoder' untouched, when the configuration is not supported */
    bool CreateEncoder( const CVoiceCodecConfig& config, TVoiceEncoderPtr& encoder );

    /** Makes a decoder, prepared on this thread. False, and 'decoder' untouched, when the configuration is not supported */
    bool CreateDecoder( const CVoiceCodecConfig& config, TVoiceDecoderPtr& decoder );

    /** Encoders and decoders made by this factory that are still alive */
    UInt32 GetLiveInstanceCount( void ) const;

    protected:

    /** Makes the encoder object for a configuration that IsSupported(), or null. The factory links it to itself for counting */
    virtual CVoiceEncoder* OnCreateEncoder( const CVoiceCodecConfig& config ) = 0;

    /** Makes the decoder object for a configuration that IsSupported(), or null. The factory links it to itself for counting */
    virtual CVoiceDecoder* OnCreateDecoder( const CVoiceCodecConfig& config ) = 0;

    private:
    friend class CVoiceEncoder;
    friend class CVoiceDecoder;

    CVoiceCodecFactory( const CVoiceCodecFactory& src );                /**< not implemented */
    CVoiceCodecFactory& operator=( const CVoiceCodecFactory& src );     /**< not implemented */

    void OnInstanceDestroyed( void );

    private:

    MT::CTAtomic< UInt32 > m_liveInstances;
};

typedef CVoiceCodecFactory::TVoiceCodecFactoryPtr TVoiceCodecFactoryPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICECODECFACTORY_H ? */
