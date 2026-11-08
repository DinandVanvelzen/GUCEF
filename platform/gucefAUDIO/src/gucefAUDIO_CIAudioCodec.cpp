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

#include "gucefAUDIO_CIAudioCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CIAudioCodec::AudioCodecFamilyName = "AudioCodec";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIAudioCodec::CIAudioCodec( void )
    : CORE::CICodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAudioCodec::CIAudioCodec( const CIAudioCodec& src )
    : CORE::CICodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAudioCodec&
CIAudioCodec::operator=( const CIAudioCodec& src )
{GUCEF_TRACE;

    CORE::CICodec::operator=( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CIAudioCodec::~CIAudioCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::OpenSource( CORE::CIOAccess& encodedInput, TAudioSourcePtr& source, const CORE::CValueList& options )
{GUCEF_TRACE;

    return OpenSourceImpl( &encodedInput, TIOAccessPtr(), options, source );
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::OpenSource( const TIOAccessPtr& encodedInput, TAudioSourcePtr& source, const CORE::CValueList& options )
{GUCEF_TRACE;

    if ( encodedInput.IsNULL() )
        return false;
    TIOAccessPtr input( encodedInput );
    return OpenSourceImpl( input.GetPointerAlways(), input, options, source );
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::DecodeAll( CORE::CIOAccess& encodedInput, CAudioBuffer& audio, const CORE::CValueList& options )
{GUCEF_TRACE;

    TAudioSourcePtr source;
    if ( !OpenSource( encodedInput, source, options ) || source.IsNULL() )
        return false;

    const CAudioFormat format = source->GetFormat();
    if ( !format.IsValid() )
        return false;

    CAudioBuffer result( format );
    const UInt32 chunkFrames = 4096;
    UInt32 total = 0;
    UInt32 stalls = 0;
    while ( !source->IsEndOfStream() )
    {
        if ( !result.Resize( total + chunkFrames ) )
            return false;
        const UInt32 read = source->ReadFrames( result.GetDataPtr( total ), chunkFrames );
        total += read;
        if ( 0 == read )
        {
            // nothing came but the source says it is not over: a source that is waiting for something. Do not wait here
            if ( ++stalls > 2 )
                break;
        }
        else
            stalls = 0;
    }
    result.Resize( total );
    audio = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::Probe( CORE::CIOAccess& encodedInput, UInt32& confidence )
{GUCEF_TRACE;

    confidence = 0;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::CanEncode( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::Encode( const CAudioBuffer&, CORE::CIOAccess&, const CORE::CValueList& )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest )
{GUCEF_TRACE;

    CAudioBuffer audio;
    if ( !DecodeAll( source, audio ) )
        return false;
    if ( audio.IsEmpty() )
        return true;
    return audio.GetSizeInBytes() == dest.Write( audio.GetDataPtr(), 1, audio.GetSizeInBytes() );
}

/*-------------------------------------------------------------------------*/

bool
CIAudioCodec::Encode( CORE::CIOAccess&, CORE::CIOAccess& )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CString
CIAudioCodec::GetFamilyName( void ) const
{GUCEF_TRACE;

    return AudioCodecFamilyName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
