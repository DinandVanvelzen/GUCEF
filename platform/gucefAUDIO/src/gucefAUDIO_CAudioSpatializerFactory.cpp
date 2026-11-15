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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioSpatializerFactory::CAudioSpatializerFactory( void )
    : m_liveInstances( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSpatializerFactory::~CAudioSpatializerFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TAudioSpatializerPtr
CAudioSpatializerFactory::Create( void )
{GUCEF_TRACE;

    IAudioSpatializer* spatializer = OnCreate();
    if ( GUCEF_NULL == spatializer )
        return TAudioSpatializerPtr();
    m_liveInstances.FetchAdd( 1, MT::MEMORDER_ACQ_REL );
    spatializer->m_factory = this;
    return TAudioSpatializerPtr( spatializer );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSpatializerFactory::GetLiveInstanceCount( void ) const
{GUCEF_TRACE;

    return m_liveInstances.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatializerFactory::OnInstanceDestroyed( void )
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
