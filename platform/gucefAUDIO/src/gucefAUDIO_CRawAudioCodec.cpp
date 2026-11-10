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

#ifndef GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#include "gucefAUDIO_CPcmStreamSource.h"
#define GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#endif /* GUCEF_AUDIO_CPCMSTREAMSOURCE_H ? */

#include "gucefAUDIO_CRawAudioCodec.h"

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

CRawAudioCodec::CRawAudioCodec( void )
    : CIAudioCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRawAudioCodec::CRawAudioCodec( const CRawAudioCodec& src )
    : CIAudioCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRawAudioCodec::~CRawAudioCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRawAudioCodec::OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TAudioSourcePtr& source )
{GUCEF_TRACE;

    if ( GUCEF_NULL == input )
        return false;

    const UInt32 sampleRate = options.GetValueAlways( "sampleRate" ).AsUInt32( 44100 );
    const UInt32 channels = options.GetValueAlways( "channels" ).AsUInt32( 1 );
    const TSampleType sampleType = CAudioFormat::SampleTypeFromString( options.GetValueAlways( "sampleType" ).AsString( "int16" ) );
    const UInt32 layout = options.GetValueAlways( "channelLayout" ).AsUInt32( 0 );
    const bool bigEndian = options.GetValueAlways( "bigEndian" ).AsBool( false );
    const UInt32 headerBytes = options.GetValueAlways( "headerBytes" ).AsUInt32( 0 );
    const UInt64 frames = options.GetValueAlways( "frames" ).AsUInt64( GUCEF_AUDIO_LENGTH_UNKNOWN );

    CAudioFormat format( sampleRate, channels, sampleType );
    if ( 0 != layout && CAudioFormat::GetChannelCountOfLayout( layout ) == channels )
        format.SetChannelLayout( layout );
    if ( !format.IsValid() )
        return false;

    const UInt64 start = input->Tell() + headerBytes;
    source = TAudioSourcePtr( GUCEF_NEW CPcmStreamSource( input, owner, format, start, frames, bigEndian ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRawAudioCodec::DecodeMetaData( CORE::CIOAccess&, CORE::CValueList& )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRawAudioCodec::CanEncode( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRawAudioCodec::Encode( const CAudioBuffer& audio, CORE::CIOAccess& encodedOutput, const CORE::CValueList& options )
{GUCEF_TRACE;

    if ( !audio.GetFormat().IsValid() )
        return false;
    if ( audio.IsEmpty() )
        return true;

    const UInt32 bytes = audio.GetSizeInBytes();
    const bool bigEndian = options.GetValueAlways( "bigEndian" ).AsBool( false );
    if ( bigEndian != CPcmStreamSource::IsHostBigEndian() && audio.GetFormat().GetBytesPerSample() > 1 )
    {
        CAudioBuffer swapped( audio );
        CPcmStreamSource::SwapSampleBytes( swapped.GetDataPtr(), swapped.GetFrameCount(), swapped.GetFormat() );
        return bytes == encodedOutput.Write( swapped.GetDataPtr(), 1, bytes );
    }
    return bytes == encodedOutput.Write( audio.GetDataPtr(), 1, bytes );
}

/*-------------------------------------------------------------------------*/

CString
CRawAudioCodec::GetType( void ) const
{GUCEF_TRACE;

    return "raw";
}

/*-------------------------------------------------------------------------*/

const CString&
CRawAudioCodec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::AUDIO::CRawAudioCodec";
    return typeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRawAudioCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRawAudioCodec( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
