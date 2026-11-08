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

#ifndef GUCEF_AUDIO_CPCMVOICEENCODER_H
#include "gucefAUDIO_CPcmVoiceEncoder.h"
#define GUCEF_AUDIO_CPCMVOICEENCODER_H
#endif /* GUCEF_AUDIO_CPCMVOICEENCODER_H ? */

#ifndef GUCEF_AUDIO_CPCMVOICEDECODER_H
#include "gucefAUDIO_CPcmVoiceDecoder.h"
#define GUCEF_AUDIO_CPCMVOICEDECODER_H
#endif /* GUCEF_AUDIO_CPCMVOICEDECODER_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#include "gucefAUDIO_CPcmVoiceCodecFactory.h"

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

CPcmVoiceCodecFactory::CPcmVoiceCodecFactory( void )
    : CVoiceCodecFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPcmVoiceCodecFactory::~CPcmVoiceCodecFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CPcmVoiceCodecFactory::GetCodecTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "pcm";
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
CPcmVoiceCodecFactory::IsSupported( const CVoiceCodecConfig& config ) const
{GUCEF_TRACE;

    if ( 1 != config.GetChannels() )
        return false;

    const UInt32 rate = config.GetSampleRate();
    if ( rate < 8000 || rate > 48000 )
        return false;

    const UInt32 duration = config.GetFrameDurationInUs();
    if ( 2500 != duration && 5000 != duration && 10000 != duration && 20000 != duration )
        return false;

    // a whole number of samples per frame
    if ( 0 != ( (UInt64) rate * duration ) % 1000000ULL )
        return false;

    return config.GetFrameSizeInFrames() * 2 <= CVoiceFrame::MaxPayloadSize;
}

/*-------------------------------------------------------------------------*/

CVoiceEncoder*
CPcmVoiceCodecFactory::OnCreateEncoder( const CVoiceCodecConfig& config )
{GUCEF_TRACE;

    return GUCEF_NEW CPcmVoiceEncoder( config );
}

/*-------------------------------------------------------------------------*/

CVoiceDecoder*
CPcmVoiceCodecFactory::OnCreateDecoder( const CVoiceCodecConfig& config )
{GUCEF_TRACE;

    return GUCEF_NEW CPcmVoiceDecoder( config );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
