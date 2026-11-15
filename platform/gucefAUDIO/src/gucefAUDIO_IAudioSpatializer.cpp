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

#include "gucefAUDIO_IAudioSpatializer.h"

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

SSpatialFrame::SSpatialFrame( void )
    : azimuth( 0.0f )
    , elevation( 0.0f )
    , distance( 0.0f )
    , pan( 0.0f )
    , spreadFactor( 1.0f )
    , gain( 1.0f )
    , rearGain( 1.0f )
    , lfeGain( 0.0f )
    , useCenterSpeaker( true )
{

}

/*-------------------------------------------------------------------------*/

IAudioSpatializer::IAudioSpatializer( void )
    : m_factory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

IAudioSpatializer::~IAudioSpatializer()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_factory )
        m_factory->OnInstanceDestroyed();
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAudioSpatializerFactory*
IAudioSpatializer::GetFactory( void ) const
{GUCEF_TRACE;

    return m_factory;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
