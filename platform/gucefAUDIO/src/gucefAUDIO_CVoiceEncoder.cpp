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

#ifndef GUCEF_AUDIO_CVOICECODECFACTORY_H
#include "gucefAUDIO_CVoiceCodecFactory.h"
#define GUCEF_AUDIO_CVOICECODECFACTORY_H
#endif /* GUCEF_AUDIO_CVOICECODECFACTORY_H ? */

#include "gucefAUDIO_CVoiceEncoder.h"

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

CVoiceEncoder::CVoiceEncoder( void )
    : m_factory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceEncoder::~CVoiceEncoder()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_factory )
        m_factory->OnInstanceDestroyed();
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceEncoder::GetFrameSizeInFrames( void ) const
{
    return GetConfig().GetFrameSizeInFrames();
}

/*-------------------------------------------------------------------------*/

void
CVoiceEncoder::SetBitrate( UInt32 bitsPerSecond )
{
    // codecs without a bit rate have nothing to do
}

/*-------------------------------------------------------------------------*/

void
CVoiceEncoder::SetExpectedLossPercentage( UInt32 percentage )
{
    // codecs that do not adapt to loss have nothing to do
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
