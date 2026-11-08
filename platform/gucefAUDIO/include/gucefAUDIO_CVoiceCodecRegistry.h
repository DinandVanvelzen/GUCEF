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

#ifndef GUCEF_AUDIO_CVOICECODECREGISTRY_H
#define GUCEF_AUDIO_CVOICECODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

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

class CAudioGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  All voice codecs that can be used, by codec type name (without regard for case). CAudioGlobal registers the
 *  built-in codec (pcm), voice codec plugins register their factories when they load and unregister them with
 *  TryUnregisterFactory() when they unload. Reach it through CAudioGlobal. Never used on the audio thread (it takes the
 *  registry lock): encoders and decoders are made on the owner thread.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceCodecRegistry : public CORE::CTGlobalONRegistry< CVoiceCodecFactory, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CVoiceCodecFactory, MT::CMutex >::TRegisteredObjPtr   TRegisteredFactoryPtr;

    /** Registers a factory under its codec type name. False when the name is taken (it stays with its first owner) */
    bool RegisterFactory( const TVoiceCodecFactoryPtr& factory );

    /**
     *  Unregisters the factory of a codec, unless encoders or decoders it made are still alive: their code may be in the
     *  plugin that wants to unload. False (and a log line) in that case, or when the codec is not registered
     */
    bool TryUnregisterFactory( const CString& codecTypeName );

    bool TryFindFactory( const CString& codecTypeName, TRegisteredFactoryPtr& factory ) const;

    /** Makes an encoder for the codec named in the configuration. False when the codec is unknown or does not support the configuration */
    bool CreateEncoder( const CVoiceCodecConfig& config, TVoiceEncoderPtr& encoder ) const;

    /** Makes a decoder for the codec named in the configuration. False when the codec is unknown or does not support the configuration */
    bool CreateDecoder( const CVoiceCodecConfig& config, TVoiceDecoderPtr& decoder ) const;

    private:
    friend class CAudioGlobal;

    CVoiceCodecRegistry( void );

    virtual ~CVoiceCodecRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CVoiceCodecRegistry( const CVoiceCodecRegistry& src );              /**< not implemented, don't use */
    CVoiceCodecRegistry& operator=( const CVoiceCodecRegistry& src );   /**< not implemented, don't use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICECODECREGISTRY_H ? */
