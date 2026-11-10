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

#ifndef GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#define GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

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
 *  Makes effects of one type. Registered in CAudioEffectRegistry under its type name, by CAudioGlobal for the built-in
 *  effects and by effect plugins (C++ link-back plugins, as audio drivers are) for theirs.
 *
 *  A factory counts the effects it made that are still alive. A plugin must not be unloaded while any of its effects
 *  live, because their code is in the plugin: CAudioEffectRegistry::TryUnregisterFactory() refuses in that case, and
 *  the factory must outlive every effect it made.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEffectFactory
{
    public:

    typedef CORE::CTSharedPtr< CAudioEffectFactory, MT::CMutex > TAudioEffectFactoryPtr;

    CAudioEffectFactory( void );

    virtual ~CAudioEffectFactory();

    /** Type name of the effects this factory makes, the key in the registry */
    virtual const CString& GetEffectTypeName( void ) const = 0;

    /** The parameters every effect of this type has, so editors can be built without making an effect */
    virtual UInt32 GetParameterCount( void ) const = 0;
    virtual const CAudioEffectParameterInfo* GetParameterInfo( UInt32 parameterId ) const = 0;

    /** Makes an effect, not yet prepared. Null when the options are not acceptable */
    TAudioEffectPtr Create( const CORE::CValueList& options = CORE::CValueList() );

    /** Effects made by this factory that are still alive */
    UInt32 GetLiveInstanceCount( void ) const;

    protected:

    /** Makes the effect object. The factory links it to itself for counting */
    virtual CAudioEffect* OnCreate( const CORE::CValueList& options ) = 0;

    private:
    friend class CAudioEffect;

    CAudioEffectFactory( const CAudioEffectFactory& src );              /**< not implemented */
    CAudioEffectFactory& operator=( const CAudioEffectFactory& src );   /**< not implemented */

    void OnInstanceDestroyed( void );

    private:

    MT::CTAtomic< UInt32 > m_liveInstances;
};

typedef CAudioEffectFactory::TAudioEffectFactoryPtr TAudioEffectFactoryPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H ? */
