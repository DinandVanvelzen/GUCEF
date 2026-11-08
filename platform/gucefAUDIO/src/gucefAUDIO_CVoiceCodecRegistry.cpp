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

#include "gucefAUDIO_CVoiceCodecRegistry.h"

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

CVoiceCodecRegistry::CVoiceCodecRegistry( void )
    : CORE::CTGlobalONRegistry< CVoiceCodecFactory, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceCodecRegistry::~CVoiceCodecRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecRegistry::RegisterFactory( const TVoiceCodecFactoryPtr& factory )
{GUCEF_TRACE;

    if ( factory.IsNULL() || factory->GetCodecTypeName().IsNULLOrEmpty() )
        return false;
    return TryRegister( factory->GetCodecTypeName().Lowercase(), factory );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecRegistry::TryUnregisterFactory( const CString& codecTypeName )
{GUCEF_TRACE;

    // under the registry lock, so nothing can be made through the registry between the check and the removal
    MT::CObjectScopeLock lock( AsLockable() );

    TRegisteredFactoryPtr factory;
    if ( !TryLookup( codecTypeName.Lowercase(), factory ) || factory.IsNULL() )
        return false;

    const UInt32 live = factory->GetLiveInstanceCount();
    if ( live > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "VoiceCodecRegistry: the voice codec \"" << codecTypeName << "\" cannot be unregistered while " << live << " of its encoders and decoders exist" );
        return false;
    }
    return TryUnregister( codecTypeName.Lowercase() );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecRegistry::TryFindFactory( const CString& codecTypeName, TRegisteredFactoryPtr& factory ) const
{GUCEF_TRACE;

    return TryLookup( codecTypeName.Lowercase(), factory );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecRegistry::CreateEncoder( const CVoiceCodecConfig& config, TVoiceEncoderPtr& encoder ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );

    TRegisteredFactoryPtr factory;
    if ( !TryLookup( config.GetCodec().Lowercase(), factory ) || factory.IsNULL() )
        return false;
    return factory->CreateEncoder( config, encoder );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecRegistry::CreateDecoder( const CVoiceCodecConfig& config, TVoiceDecoderPtr& decoder ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );

    TRegisteredFactoryPtr factory;
    if ( !TryLookup( config.GetCodec().Lowercase(), factory ) || factory.IsNULL() )
        return false;
    return factory->CreateDecoder( config, decoder );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
