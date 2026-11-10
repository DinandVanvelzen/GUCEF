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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioEffectRegistry::CAudioEffectRegistry( void )
    : CORE::CTGlobalONRegistry< CAudioEffectFactory, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEffectRegistry::~CAudioEffectRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectRegistry::RegisterFactory( const TAudioEffectFactoryPtr& factory )
{GUCEF_TRACE;

    if ( factory.IsNULL() || factory->GetEffectTypeName().IsNULLOrEmpty() )
        return false;
    return TryRegister( factory->GetEffectTypeName().Lowercase(), factory );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectRegistry::TryUnregisterFactory( const CString& typeName )
{GUCEF_TRACE;

    // under the registry lock, so no effect can be made through the registry between the check and the removal
    MT::CObjectScopeLock lock( AsLockable() );

    TRegisteredFactoryPtr factory;
    if ( !TryLookup( typeName.Lowercase(), factory ) || factory.IsNULL() )
        return false;

    const UInt32 live = factory->GetLiveInstanceCount();
    if ( live > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AudioEffectRegistry: the effect type \"" << typeName << "\" cannot be unregistered while " << live << " of its effects exist" );
        return false;
    }
    return TryUnregister( typeName.Lowercase() );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectRegistry::TryFindFactory( const CString& typeName, TRegisteredFactoryPtr& factory ) const
{GUCEF_TRACE;

    return TryLookup( typeName.Lowercase(), factory );
}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioEffectRegistry::CreateEffect( const CString& typeName, const CORE::CValueList& options ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );

    TRegisteredFactoryPtr factory;
    if ( !TryLookup( typeName.Lowercase(), factory ) || factory.IsNULL() )
        return TAudioEffectPtr();
    return factory->Create( options );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
