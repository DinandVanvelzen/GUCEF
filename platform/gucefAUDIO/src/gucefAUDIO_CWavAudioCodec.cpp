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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#include "gucefAUDIO_CPcmStreamSource.h"
#define GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#endif /* GUCEF_AUDIO_CPCMSTREAMSOURCE_H ? */

#include "gucefAUDIO_CWavAudioCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt16 WaveFormatPcm = 1;
const UInt16 WaveFormatFloat = 3;
const UInt16 WaveFormatExtensible = 0xFFFE;

inline UInt16 LoadLE16( const UInt8* p ) { return (UInt16) ( p[ 0 ] | ( p[ 1 ] << 8 ) ); }
inline UInt32 LoadLE32( const UInt8* p ) { return (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 ); }

inline void StoreLE16( UInt8* p, UInt16 v ) { p[ 0 ] = (UInt8) ( v & 0xFF ); p[ 1 ] = (UInt8) ( v >> 8 ); }
inline void StoreLE32( UInt8* p, UInt32 v ) { p[ 0 ] = (UInt8) ( v & 0xFF ); p[ 1 ] = (UInt8) ( ( v >> 8 ) & 0xFF ); p[ 2 ] = (UInt8) ( ( v >> 16 ) & 0xFF ); p[ 3 ] = (UInt8) ( v >> 24 ); }

bool
SkipBytes( CORE::CIOAccess& input, UInt64 count )
{
    if ( 0 == count )
        return true;
    if ( 0 == input.Seek( (Int64) count, SEEK_CUR ) )
        return true;

    // an input that cannot seek is read through
    UInt8 scratch[ 512 ];
    while ( count > 0 )
    {
        const UInt32 want = ( count < sizeof( scratch ) ) ? (UInt32) count : (UInt32) sizeof( scratch );
        const UInt32 got = input.Read( scratch, 1, want );
        if ( 0 == got )
            return false;
        count -= got;
    }
    return true;
}

bool
SampleTypeFromWave( UInt16 tag, UInt16 bits, TSampleType& type )
{
    if ( WaveFormatPcm == tag )
    {
        switch ( bits )
        {
            case 8  : type = SAMPLETYPE_UINT8; return true;
            case 16 : type = SAMPLETYPE_INT16; return true;
            case 24 : type = SAMPLETYPE_INT24; return true;
            case 32 : type = SAMPLETYPE_INT32; return true;
            default : return false;
        }
    }
    if ( WaveFormatFloat == tag )
    {
        if ( 32 == bits ) { type = SAMPLETYPE_FLOAT32; return true; }
        if ( 64 == bits ) { type = SAMPLETYPE_FLOAT64; return true; }
    }
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWavAudioCodec::CWavAudioCodec( void )
    : CIAudioCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWavAudioCodec::CWavAudioCodec( const CWavAudioCodec& src )
    : CIAudioCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWavAudioCodec::~CWavAudioCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWavAudioCodec::ParseHeader( CORE::CIOAccess& input, SWavInfo& info )
{GUCEF_TRACE;

    info.format = CAudioFormat();
    info.dataOffset = 0;
    info.frameCount = GUCEF_AUDIO_LENGTH_UNKNOWN;

    UInt8 riff[ 12 ];
    if ( 12 != input.Read( riff, 1, 12 ) || 0 != memcmp( riff, "RIFF", 4 ) || 0 != memcmp( riff + 8, "WAVE", 4 ) )
        return false;

    bool haveFormat = false;
    CAudioFormat format;
    UInt32 blockAlign = 0;
    for ( ;; )
    {
        UInt8 chunkHeader[ 8 ];
        if ( 8 != input.Read( chunkHeader, 1, 8 ) )
            return false;
        const UInt32 chunkSize = LoadLE32( chunkHeader + 4 );

        if ( 0 == memcmp( chunkHeader, "fmt ", 4 ) )
        {
            if ( chunkSize < 16 )
                return false;
            UInt8 fmt[ 40 ];
            const UInt32 toRead = ( chunkSize < sizeof( fmt ) ) ? chunkSize : (UInt32) sizeof( fmt );
            if ( toRead != input.Read( fmt, 1, toRead ) )
                return false;

            UInt16 tag = LoadLE16( fmt );
            const UInt16 channels = LoadLE16( fmt + 2 );
            const UInt32 rate = LoadLE32( fmt + 4 );
            blockAlign = LoadLE16( fmt + 12 );
            const UInt16 bits = LoadLE16( fmt + 14 );
            UInt32 layout = 0;
            if ( WaveFormatExtensible == tag )
            {
                if ( toRead < 40 )
                    return false;
                layout = LoadLE32( fmt + 20 );
                tag = LoadLE16( fmt + 24 );         // the sub format starts with the format tag
            }

            TSampleType type = SAMPLETYPE_UNKNOWN;
            if ( 0 == channels || 0 == rate || !SampleTypeFromWave( tag, bits, type ) )
                return false;
            if ( blockAlign != (UInt32) channels * ( bits / 8 ) )
                return false;

            format = CAudioFormat( rate, channels, type );
            if ( 0 != layout && CAudioFormat::GetChannelCountOfLayout( layout ) == channels )
                format.SetChannelLayout( layout );
            haveFormat = true;

            if ( !SkipBytes( input, ( chunkSize - toRead ) + ( chunkSize & 1 ) ) )
                return false;
        }
        else if ( 0 == memcmp( chunkHeader, "data", 4 ) )
        {
            if ( !haveFormat )
                return false;
            info.format = format;
            info.dataOffset = input.Tell();
            // A size of zero or all ones is what streaming writers leave when they cannot go back to fill it in
            if ( 0 != chunkSize && 0xFFFFFFFFU != chunkSize )
                info.frameCount = chunkSize / blockAlign;
            return true;
        }
        else
        {
            if ( !SkipBytes( input, (UInt64) chunkSize + ( chunkSize & 1 ) ) )
                return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CWavAudioCodec::OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TAudioSourcePtr& source )
{GUCEF_TRACE;

    if ( GUCEF_NULL == input )
        return false;

    SWavInfo info;
    if ( !ParseHeader( *input, info ) )
        return false;

    // WAVE stores samples with the least significant byte first
    source = TAudioSourcePtr( GUCEF_NEW CPcmStreamSource( input, owner, info.format, info.dataOffset, info.frameCount, false ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWavAudioCodec::DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData )
{GUCEF_TRACE;

    SWavInfo info;
    if ( !ParseHeader( encodedInput, info ) )
        return false;

    metaData.Set( "sampleRate", CORE::UInt32ToString( info.format.GetSampleRate() ) );
    metaData.Set( "channels", CORE::UInt32ToString( info.format.GetChannelCount() ) );
    metaData.Set( "sampleType", CString( CAudioFormat::SampleTypeToString( info.format.GetSampleType() ) ) );
    metaData.Set( "channelLayout", CORE::UInt32ToString( info.format.GetChannelLayout() ) );
    if ( GUCEF_AUDIO_LENGTH_UNKNOWN != info.frameCount )
    {
        metaData.Set( "frames", CORE::UInt64ToString( info.frameCount ) );
        metaData.Set( "durationInMs", CORE::UInt64ToString( info.frameCount * 1000 / info.format.GetSampleRate() ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWavAudioCodec::CanEncode( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWavAudioCodec::Encode( const CAudioBuffer& audio, CORE::CIOAccess& encodedOutput, const CORE::CValueList& options )
{GUCEF_TRACE;

    const CAudioFormat& format = audio.GetFormat();
    if ( !format.IsValid() || format.GetChannelCount() > 0xFFFF )
        return false;

    const UInt32 bitsPerSample = format.GetBytesPerSample() * 8;
    const bool isFloat = SAMPLETYPE_FLOAT32 == format.GetSampleType() || SAMPLETYPE_FLOAT64 == format.GetSampleType();
    const UInt32 dataBytes = audio.GetSizeInBytes();
    // The plain header is understood by every player and says everything for mono and stereo. The extensible
    // header is for more channels or speakers that are not the usual ones. Float formats carry a size field for extra data, zero here
    const bool extensible = format.GetChannelCount() > 2 || format.GetChannelLayout() != CAudioFormat::GetDefaultChannelLayout( format.GetChannelCount() );
    const UInt32 fmtSize = extensible ? 40 : ( isFloat ? 18 : 16 );
    const UInt64 riffSize = 4 + ( 8 + (UInt64) fmtSize ) + ( 8 + (UInt64) dataBytes + ( dataBytes & 1 ) );
    if ( riffSize > 0xFFFFFFFFULL )
        return false;               // beyond what a plain WAVE file can hold

    UInt8 header[ 12 + 8 + 40 + 8 ];
    UInt32 at = 0;
    memcpy( header + at, "RIFF", 4 ); at += 4;
    StoreLE32( header + at, (UInt32) riffSize ); at += 4;
    memcpy( header + at, "WAVE", 4 ); at += 4;
    memcpy( header + at, "fmt ", 4 ); at += 4;
    StoreLE32( header + at, fmtSize ); at += 4;
    StoreLE16( header + at, extensible ? WaveFormatExtensible : ( isFloat ? WaveFormatFloat : WaveFormatPcm ) ); at += 2;
    StoreLE16( header + at, (UInt16) format.GetChannelCount() ); at += 2;
    StoreLE32( header + at, format.GetSampleRate() ); at += 4;
    StoreLE32( header + at, format.GetSampleRate() * format.GetBytesPerFrame() ); at += 4;
    StoreLE16( header + at, (UInt16) format.GetBytesPerFrame() ); at += 2;
    StoreLE16( header + at, (UInt16) bitsPerSample ); at += 2;
    if ( !extensible && isFloat )
    {
        StoreLE16( header + at, 0 ); at += 2;                           // no extra format data
    }
    if ( extensible )
    {
        StoreLE16( header + at, 22 ); at += 2;                          // size of the rest
        StoreLE16( header + at, (UInt16) bitsPerSample ); at += 2;      // valid bits
        StoreLE32( header + at, format.GetChannelLayout() ); at += 4;
        // sub format GUID: the format tag followed by the fixed tail 00000000-0010-8000-00AA00389B71
        static const UInt8 guidTail[ 14 ] = { 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };
        StoreLE16( header + at, isFloat ? WaveFormatFloat : WaveFormatPcm ); at += 2;
        memcpy( header + at, guidTail, 14 ); at += 14;
    }
    memcpy( header + at, "data", 4 ); at += 4;
    StoreLE32( header + at, dataBytes ); at += 4;

    if ( at != encodedOutput.Write( header, 1, at ) )
        return false;

    if ( dataBytes > 0 )
    {
        if ( CPcmStreamSource::IsHostBigEndian() && format.GetBytesPerSample() > 1 )
        {
            // the file wants the least significant byte first
            CAudioBuffer swapped( audio );
            CPcmStreamSource::SwapSampleBytes( swapped.GetDataPtr(), swapped.GetFrameCount(), format );
            if ( dataBytes != encodedOutput.Write( swapped.GetDataPtr(), 1, dataBytes ) )
                return false;
        }
        else if ( dataBytes != encodedOutput.Write( audio.GetDataPtr(), 1, dataBytes ) )
            return false;
    }
    if ( dataBytes & 1 )
    {
        const UInt8 pad = 0;
        if ( 1 != encodedOutput.Write( &pad, 1, 1 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CWavAudioCodec::GetType( void ) const
{GUCEF_TRACE;

    return "wav";
}

/*-------------------------------------------------------------------------*/

const CString&
CWavAudioCodec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::AUDIO::CWavAudioCodec";
    return typeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CWavAudioCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CWavAudioCodec( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CWavAudioCodec::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CWavAudioCodec::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::AUDIO::CWavAudioCodec";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
