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

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#include "gucefAUDIO_CAudioEffectFactory.h"
#define GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H ? */

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
 *  All effect types that can be made, by type name (without regard for case). CAudioGlobal registers the built-in effects,
 *  effect plugins register their factories when they load and unregister them with TryUnregisterFactory() when they
 *  unload. Reach it through CAudioGlobal. Never used on the audio thread (it takes the registry lock).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEffectRegistry : public CORE::CTGlobalONRegistry< CAudioEffectFactory, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CAudioEffectFactory, MT::CMutex >::TRegisteredObjPtr   TRegisteredFactoryPtr;

    /** Registers a factory under its effect type name. False when the name is taken (it stays with its first owner) */
    bool RegisterFactory( const TAudioEffectFactoryPtr& factory );

    /**
     *  Unregisters the factory of a type, unless effects it made are still alive: their code may be in the plugin that
     *  wants to unload. False (and a log line) in that case, or when the type is not registered
     */
    bool TryUnregisterFactory( const CString& typeName );

    bool TryFindFactory( const CString& typeName, TRegisteredFactoryPtr& factory ) const;

    /** Makes an effect of a type, not yet prepared. Null for an unknown type */
    TAudioEffectPtr CreateEffect( const CString& typeName, const CORE::CValueList& options = CORE::CValueList() ) const;

    private:
    friend class CAudioGlobal;

    CAudioEffectRegistry( void );

    virtual ~CAudioEffectRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CAudioEffectRegistry( const CAudioEffectRegistry& src );            /**< not implemented, don't use */
    CAudioEffectRegistry& operator=( const CAudioEffectRegistry& src ); /**< not implemented, don't use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */
