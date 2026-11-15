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

#ifndef GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H
#include "gucefAUDIO_CAudioSpatializerRegistry.h"
#define GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H ? */

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

CAudioSpatializerRegistry::CAudioSpatializerRegistry( void )
    : CORE::CTGlobalONRegistry< CAudioSpatializerFactory, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSpatializerRegistry::~CAudioSpatializerRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatializerRegistry::RegisterFactory( const TAudioSpatializerFactoryPtr& factory, bool builtIn )
{GUCEF_TRACE;

    if ( factory.IsNULL() || factory->GetSpatializerTypeName().IsNULLOrEmpty() )
        return false;
    const CString name = factory->GetSpatializerTypeName().Lowercase();
    return builtIn ? TryRegisterBuiltIn( name, factory ) : TryRegister( name, factory );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatializerRegistry::TryUnregisterFactory( const CString& typeName )
{GUCEF_TRACE;

    // under the registry lock, so no spatializer can be made through the registry between the check and the removal
    MT::CObjectScopeLock lock( AsLockable() );
    TRegisteredFactoryPtr factory;
    if ( !TryLookup( typeName.Lowercase(), factory ) || factory.IsNULL() )
        return false;
    const UInt32 live = factory->GetLiveInstanceCount();
    if ( live > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AudioSpatializerRegistry: the spatializer type \"" << typeName << "\" cannot be unregistered while " << live << " of its spatializers exist" );
        return false;
    }
    return TryUnregister( typeName.Lowercase() );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatializerRegistry::TryFindFactory( const CString& typeName, TRegisteredFactoryPtr& factory ) const
{GUCEF_TRACE;

    return TryLookup( typeName.Lowercase(), factory );
}

/*-------------------------------------------------------------------------*/

TAudioSpatializerPtr
CAudioSpatializerRegistry::CreateSpatializer( const CString& typeName ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );
    TRegisteredFactoryPtr factory;
    if ( !TryLookup( typeName.Lowercase(), factory ) || factory.IsNULL() )
        return TAudioSpatializerPtr();
    return factory->Create();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
