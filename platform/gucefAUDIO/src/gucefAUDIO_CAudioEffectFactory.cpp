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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioEffectFactory::CAudioEffectFactory( void )
    : m_liveInstances( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEffectFactory::~CAudioEffectFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioEffectFactory::Create( const CORE::CValueList& options )
{GUCEF_TRACE;

    CAudioEffect* effect = OnCreate( options );
    if ( GUCEF_NULL == effect )
        return TAudioEffectPtr();

    m_liveInstances.FetchAdd( 1, MT::MEMORDER_ACQ_REL );
    effect->m_factory = this;
    return TAudioEffectPtr( effect );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffectFactory::GetLiveInstanceCount( void ) const
{GUCEF_TRACE;

    return m_liveInstances.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectFactory::OnInstanceDestroyed( void )
{GUCEF_TRACE;

    m_liveInstances.FetchSub( 1, MT::MEMORDER_ACQ_REL );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
