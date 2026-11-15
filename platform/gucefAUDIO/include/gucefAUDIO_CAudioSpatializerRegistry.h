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

#ifndef GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H
#define GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H
#include "gucefAUDIO_CAudioSpatializerFactory.h"
#define GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H ? */

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

/**
 *  All spatializer types that can be made, by type name (without regard for case), the same in shape as
 *  CAudioEffectRegistry (design 5.9). CAudioGlobal registers the built-in panning spatializer ("panning"), spatializer
 *  plugins register their factories when they load and unregister them with TryUnregisterFactory() when they unload.
 *  The engine makes the spatializer of a spatial voice with the name of CAudioSpatialParams::GetSpatializerName() when the
 *  voice is started. Reach it through CAudioGlobal. Never used on the audio thread (it takes the registry lock).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioSpatializerRegistry : public CORE::CTGlobalONRegistry< CAudioSpatializerFactory, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CAudioSpatializerFactory, MT::CMutex >::TRegisteredObjPtr   TRegisteredFactoryPtr;

    /** Registers a factory under its spatializer type name. False when the name is taken (it stays with its first owner) */
    bool RegisterFactory( const TAudioSpatializerFactoryPtr& factory );

    /**
     *  Unregisters the factory of a type, unless spatializers it made are still alive: their code may be in the plugin that
     *  wants to unload. False (and a log line) in that case, or when the type is not registered
     */
    bool TryUnregisterFactory( const CString& typeName );

    bool TryFindFactory( const CString& typeName, TRegisteredFactoryPtr& factory ) const;

    /** Makes a spatializer of a type, not yet prepared. Null for an unknown type */
    TAudioSpatializerPtr CreateSpatializer( const CString& typeName ) const;

    private:
    friend class CAudioGlobal;

    CAudioSpatializerRegistry( void );

    virtual ~CAudioSpatializerRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CAudioSpatializerRegistry( const CAudioSpatializerRegistry& src );            /**< not implemented, don't use */
    CAudioSpatializerRegistry& operator=( const CAudioSpatializerRegistry& src ); /**< not implemented, don't use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H ? */
