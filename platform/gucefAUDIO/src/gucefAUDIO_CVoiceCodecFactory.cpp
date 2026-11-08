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

#include "gucefAUDIO_CVoiceCodecFactory.h"

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

CVoiceCodecFactory::CVoiceCodecFactory( void )
    : m_liveInstances( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceCodecFactory::~CVoiceCodecFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecFactory::CreateEncoder( const CVoiceCodecConfig& config, TVoiceEncoderPtr& encoder )
{GUCEF_TRACE;

    if ( !IsSupported( config ) )
        return false;

    CVoiceEncoder* made = OnCreateEncoder( config );
    if ( GUCEF_NULL == made )
        return false;

    m_liveInstances.FetchAdd( 1, MT::MEMORDER_ACQ_REL );
    made->m_factory = this;
    encoder = TVoiceEncoderPtr( made );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecFactory::CreateDecoder( const CVoiceCodecConfig& config, TVoiceDecoderPtr& decoder )
{GUCEF_TRACE;

    if ( !IsSupported( config ) )
        return false;

    CVoiceDecoder* made = OnCreateDecoder( config );
    if ( GUCEF_NULL == made )
        return false;

    m_liveInstances.FetchAdd( 1, MT::MEMORDER_ACQ_REL );
    made->m_factory = this;
    decoder = TVoiceDecoderPtr( made );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecFactory::GetLiveInstanceCount( void ) const
{GUCEF_TRACE;

    return m_liveInstances.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecFactory::OnInstanceDestroyed( void )
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
