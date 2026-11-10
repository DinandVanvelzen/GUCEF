/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioCodecSupport.h"
#include "TestAudioCodecFlac.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace CodecTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* PluginGroup = "audiocodecpluginflac";

enum ESignal
{
    SIGNAL_TONE = 0,
    SIGNAL_NOISE,
    SIGNAL_SILENCE,
    SIGNAL_EXTREMES,
    SIGNAL_STEPS
};

/*-------------------------------------------------------------------------*/

/** 16 bit audio of one of a few kinds, written sample by sample: the kinds exercise the sub frame types of the encoder */
AUDIO::CAudioBuffer
MakeInt16( UInt32 rate, UInt32 channels, UInt32 frames, ESignal kind, UInt32 seed = 7 )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_INT16 ), frames );
    Int16* samples = (Int16*) audio.GetDataPtr();
    CRandom random( seed );
    for ( UInt32 f=0; f<frames; ++f )
    {
        for ( UInt32 c=0; c<channels; ++c )
        {
            Int32 value = 0;
            switch ( kind )
            {
                case SIGNAL_TONE: value = (Int32) ( 20000.0 * sin( 0.05 * (Float64) ( c + 1 ) * (Float64) f ) ); break;
                case SIGNAL_NOISE: value = (Int32) ( random.Next() & 0xFFFF ) - 32768; break;
                case SIGNAL_SILENCE: value = 1234; break;
                case SIGNAL_EXTREMES: value = ( ( f + c ) & 1 ) ? 32767 : -32768; break;
                default: value = (Int32) ( ( f / 37 ) % 65536 ) - 20000 + (Int32) c * 3; break;
            }
            samples[ (size_t) f * channels + c ] = (Int16) value;
        }
    }
    return audio;
}

/*-------------------------------------------------------------------------*/

/** Encodes, decodes and compares: FLAC is lossless, so the bytes have to be the same */
bool
RoundTripIsExact( TCodecPtr flac, const AUDIO::CAudioBuffer& audio, const CORE::CValueList& options = CORE::CValueList() )
{
    CORE::CDynamicBuffer file;
    if ( !Encode( flac, audio, file, options ) )
        return false;
    AUDIO::CAudioBuffer decoded;
    if ( !DecodeFile( flac, file, decoded ) )
        return false;
    return SameBytes( audio, decoded );
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetAssetsDirectory( void )
{
    return CORE::CString( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_ASSETS" ).c_str() );
}

/*-------------------------------------------------------------------------*/

/** The names (with path) of the .flac files in a directory */
GUCEF::vector< CORE::CString >
ListFlacFiles( const CORE::CString& directory )
{
    GUCEF::vector< CORE::CString > files;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( directory ) )
    {
        do
        {
            if ( iterator.IsAFile() )
            {
                const CORE::CString name = iterator.GetResourceName();
                const CORE::CString lower = name.Lowercase();
                if ( lower.Length() > 5 && 0 == strcmp( lower.C_String() + lower.Length() - 5, ".flac" ) )
                    files.push_back( CORE::CombinePath( directory, name ) );
            }
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return files;
}

/*-------------------------------------------------------------------------*/

/** The MD5 STREAMINFO of a FLAC file keeps of the audio, false when the file has no STREAMINFO or the MD5 is not filled in */
bool
GetStreamInfoMd5( const CORE::CDynamicBuffer& file, UInt8* md5, UInt32& bitsPerSample )
{
    if ( file.GetDataSize() < 42 || 0 != memcmp( file.GetConstBufferPtr(), "fLaC", 4 ) )
        return false;
    const UInt8* bytes = (const UInt8*) file.GetConstBufferPtr();
    if ( 0 != ( bytes[ 4 ] & 0x7F ) )
        return false;
    bitsPerSample = ( ( ( (UInt32) bytes[ 20 ] & 1 ) << 4 ) | ( (UInt32) bytes[ 21 ] >> 4 ) ) + 1;
    memcpy( md5, bytes + 26, 16 );
    UInt8 any = 0;
    for ( UInt32 i=0; i<16; ++i )
        any |= md5[ i ];
    return 0 != any;
}

/*-------------------------------------------------------------------------*/

/** The MD5 over the samples of decoded audio the way FLAC defines it: signed, little endian, in as many bytes as the bits need, interleaved */
void
Md5OfDecoded( const AUDIO::CAudioBuffer& audio, UInt32 bitsPerSample, UInt8* digest )
{
    CMd5 md5;
    const UInt32 bytesPerSample = ( bitsPerSample + 7 ) / 8;
    const bool isWide = AUDIO::SAMPLETYPE_INT32 == audio.GetFormat().GetSampleType();
    const UInt32 shift = isWide ? 32 - bitsPerSample : 16 - bitsPerSample;
    const UInt32 count = audio.GetFrameCount() * audio.GetFormat().GetChannelCount();
    for ( UInt32 i=0; i<count; ++i )
    {
        const Int32 value = isWide ? ( (const Int32*) audio.GetDataPtr() )[ i ] : (Int32) ( (const Int16*) audio.GetDataPtr() )[ i ];
        const Int32 sample = value >> shift;
        UInt8 bytes[ 4 ];
        for ( UInt32 b=0; b<bytesPerSample; ++b )
            bytes[ b ] = (UInt8) ( ( (UInt32) sample >> ( 8 * b ) ) & 0xFF );
        md5.Update( bytes, bytesPerSample );
    }
    md5.Final( digest );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioCodecFlacTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING audio FLAC codec plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.CodecFlac" );

    const bool loaded = LoadPlugin( "audiocodecpluginFLAC", PluginGroup, "flac" );

    GUCEF_TESTFW_TESTCASE( "CodecFlac: The plugin loads and registers the flac type, streams of it decode at the same time" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !loaded )
        GUCEF_TESTFW_SKIP( "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( loaded );
        TCodecPtr flac = FindCodec( "flac" );
        ASSERT_FALSE( flac.IsNULL() );
        ASSERT_TRUE( flac->GetType() == "flac" );
        ASSERT_TRUE( flac->CanEncode() );
        TCodecPtr byName;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForFilename( "music/theme.FLAC", byName ) );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == GetThreadingModel( "flac" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !loaded )
        return;

    TCodecPtr flac = FindCodec( "flac" );

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Round trips are exact for every kind of signal, length and block size" )
    try
    {
        const UInt32 lengths[] = { 0, 1, 2, 15, 16, 17, 255, 256, 257, 4095, 4096, 4097, 9000 };
        const UInt32 channelCounts[] = { 1, 2, 3, 6, 8 };
        for ( UInt32 k=0; k<5; ++k )
        {
            for ( UInt32 l=0; l<sizeof( lengths ) / sizeof( lengths[ 0 ] ); ++l )
            {
                const UInt32 channels = channelCounts[ ( k + l ) % 5 ];
                const AUDIO::CAudioBuffer audio = MakeInt16( 44100, channels, lengths[ l ], (ESignal) k, 11 + l );
                ASSERT_TRUE( RoundTripIsExact( flac, audio ) );
            }
        }

        // block sizes of the table of the format and explicit ones, small and big
        const AUDIO::CAudioBuffer tone = MakeInt16( 48000, 2, 7000, SIGNAL_TONE );
        const char* blockSizes[] = { "16", "17", "192", "256", "300", "576", "1000", "4608", "8192", "65535" };
        for ( UInt32 b=0; b<sizeof( blockSizes ) / sizeof( blockSizes[ 0 ] ); ++b )
        {
            CORE::CValueList options;
            options.Set( "blocksize", blockSizes[ b ] );
            ASSERT_TRUE( RoundTripIsExact( flac, tone, options ) );
        }

        // other sample rates, up to the 20 bits the format has
        ASSERT_TRUE( RoundTripIsExact( flac, MakeInt16( 8000, 1, 3000, SIGNAL_TONE ) ) );
        ASSERT_TRUE( RoundTripIsExact( flac, MakeInt16( 96000, 2, 3000, SIGNAL_NOISE ) ) );
        ASSERT_TRUE( RoundTripIsExact( flac, MakeInt16( 655350, 1, 3000, SIGNAL_TONE ) ) );

        // the frames of the sources are 16 bit
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, tone, file ) );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( flac, file, source ) );
        ASSERT_TRUE( source->GetFormat() == AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( 7000 == source->GetLengthInFrames() );
        ASSERT_TRUE( source->IsSeekable() );

        // the encoder says no to what the format can not hold
        CORE::CDynamicBuffer rejected;
        ASSERT_FALSE( Encode( flac, MakeInt16( 44100, 9, 100, SIGNAL_TONE ), rejected ) );
        ASSERT_FALSE( Encode( flac, MakeInt16( 700000, 1, 100, SIGNAL_TONE ), rejected ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Other sample types: 24 bit as left justified 32 bit, 8 bit, and float input" )
    try
    {
        // 24 bit audio is 32 bit left justified (the low byte is zero)
        {
            AUDIO::CAudioBuffer wide( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT32 ), 6000 );
            Int32* samples = (Int32*) wide.GetDataPtr();
            CRandom random( 99 );
            for ( UInt32 i=0; i<6000 * 2; ++i )
                samples[ i ] = ( (Int32) ( random.Next() & 0xFFFFFF ) - 0x800000 ) * 256;
            samples[ 0 ] = (Int32) 0x80000000;
            samples[ 1 ] = (Int32) 0x7FFFFF00;
            ASSERT_TRUE( RoundTripIsExact( flac, wide ) );

            CORE::CDynamicBuffer file;
            ASSERT_TRUE( Encode( flac, wide, file ) );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( flac, file, source ) );
            ASSERT_TRUE( AUDIO::SAMPLETYPE_INT32 == source->GetFormat().GetSampleType() );
        }

        // 8 bit audio comes out as 16 bit, shifted up
        {
            AUDIO::CAudioBuffer small( AUDIO::CAudioFormat( 11025, 1, AUDIO::SAMPLETYPE_UINT8 ), 3000 );
            UInt8* samples = (UInt8*) small.GetDataPtr();
            for ( UInt32 i=0; i<3000; ++i )
                samples[ i ] = (UInt8) ( 128 + 100.0 * sin( 0.03 * (Float64) i ) );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( Encode( flac, small, file ) );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( DecodeFile( flac, file, decoded ) );
            ASSERT_TRUE( AUDIO::SAMPLETYPE_INT16 == decoded.GetFormat().GetSampleType() && 3000 == decoded.GetFrameCount() );
            const Int16* result = (const Int16*) decoded.GetDataPtr();
            bool isSame = true;
            for ( UInt32 i=0; i<3000; ++i )
                isSame = isSame && result[ i ] == (Int16) ( ( (Int32) samples[ i ] - 128 ) * 256 );
            ASSERT_TRUE( isSame );
        }

        // float input is written as 16 bit
        {
            AUDIO::CAudioBuffer real( AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 2000 );
            Float32* samples = (Float32*) real.GetDataPtr();
            for ( UInt32 i=0; i<2000; ++i )
                samples[ i ] = (Float32) ( ( (Int32) ( i * 31 ) % 60000 ) - 30000 ) / 32768.0f;
            samples[ 5 ] = 2.0f;        // too loud for the format: clipped
            samples[ 6 ] = -2.0f;
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( Encode( flac, real, file ) );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( DecodeFile( flac, file, decoded ) );
            const Int16* result = (const Int16*) decoded.GetDataPtr();
            ASSERT_TRUE( 2000 == decoded.GetFrameCount() );
            ASSERT_TRUE( 32767 == result[ 5 ] && -32768 == result[ 6 ] );
            bool isSame = true;
            for ( UInt32 i=0; i<2000; ++i )
                if ( 5 != i && 6 != i )
                    isSame = isSame && result[ i ] == (Int16) ( ( (Int32) ( i * 31 ) % 60000 ) - 30000 );
            ASSERT_TRUE( isSame );

            // and as 24 bit when asked for
            CORE::CValueList options;
            options.Set( "bits", "24" );
            ASSERT_TRUE( Encode( flac, real, file, options ) );
            ASSERT_TRUE( DecodeFile( flac, file, decoded ) );
            ASSERT_TRUE( AUDIO::SAMPLETYPE_INT32 == decoded.GetFormat().GetSampleType() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: STREAMINFO of the encoder is right and its MD5 is the MD5 of the audio" )
    try
    {
        // MD5 itself against RFC 1321
        {
            CMd5 md5;
            md5.Update( "abc", 3 );
            UInt8 digest[ 16 ];
            md5.Final( digest );
            const UInt8 expected[ 16 ] = { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 };
            ASSERT_TRUE( 0 == memcmp( digest, expected, 16 ) );
        }

        // a signal for which the MD5 was worked out with md5sum on the little endian samples: 100 samples of i * 37 - 1000
        {
            AUDIO::CAudioBuffer ramp( AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ), 100 );
            for ( UInt32 i=0; i<100; ++i )
                ( (Int16*) ramp.GetDataPtr() )[ i ] = (Int16) ( (Int32) i * 37 - 1000 );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( Encode( flac, ramp, file ) );
            UInt8 md5[ 16 ];
            UInt32 bits = 0;
            ASSERT_TRUE( GetStreamInfoMd5( file, md5, bits ) );
            // f1168a682f120f33a253ee7771bbc4d3
            const UInt8 known[ 16 ] = { 0xf1, 0x16, 0x8a, 0x68, 0x2f, 0x12, 0x0f, 0x33, 0xa2, 0x53, 0xee, 0x77, 0x71, 0xbb, 0xc4, 0xd3 };
            ASSERT_TRUE( 0 == memcmp( md5, known, 16 ) );
            ASSERT_TRUE( 16 == bits );
        }

        // the fields of STREAMINFO, and the MD5 of the data for several formats
        const AUDIO::CAudioBuffer stereo = MakeInt16( 44100, 2, 20000, SIGNAL_TONE );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, stereo, file ) );
        const UInt8* bytes = (const UInt8*) file.GetConstBufferPtr();
        ASSERT_TRUE( 0 == memcmp( bytes, "fLaC", 4 ) );
        ASSERT_TRUE( 34 == bytes[ 7 ] && 0 == bytes[ 5 ] && 0 == bytes[ 6 ] );             // STREAMINFO, 34 bytes
        const UInt32 rate = ( (UInt32) bytes[ 18 ] << 12 ) | ( (UInt32) bytes[ 19 ] << 4 ) | ( (UInt32) bytes[ 20 ] >> 4 );
        const UInt32 channels = ( ( (UInt32) bytes[ 20 ] >> 1 ) & 7 ) + 1;
        const UInt64 total = ( ( (UInt64) bytes[ 21 ] & 0xF ) << 32 ) | ( (UInt64) bytes[ 22 ] << 24 ) | ( (UInt64) bytes[ 23 ] << 16 ) | ( (UInt64) bytes[ 24 ] << 8 ) | bytes[ 25 ];
        ASSERT_TRUE( 44100 == rate && 2 == channels && 20000 == total );
        UInt8 stored[ 16 ];
        UInt8 computed[ 16 ];
        UInt32 bits = 0;
        ASSERT_TRUE( GetStreamInfoMd5( file, stored, bits ) );
        Md5OfDecoded( stereo, bits, computed );
        ASSERT_TRUE( 0 == memcmp( stored, computed, 16 ) );

        AUDIO::CAudioBuffer wide( AUDIO::CAudioFormat( 44100, 3, AUDIO::SAMPLETYPE_INT32 ), 5000 );
        CRandom random( 5 );
        for ( UInt32 i=0; i<5000 * 3; ++i )
            ( (Int32*) wide.GetDataPtr() )[ i ] = ( (Int32) ( random.Next() & 0xFFFFFF ) - 0x800000 ) * 256;
        ASSERT_TRUE( Encode( flac, wide, file ) );
        ASSERT_TRUE( GetStreamInfoMd5( file, stored, bits ) );
        ASSERT_TRUE( 24 == bits );
        Md5OfDecoded( wide, bits, computed );
        ASSERT_TRUE( 0 == memcmp( stored, computed, 16 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Seeking is exact: 50 random places, the start, the last frame and the end" )
    try
    {
        // noise on a tone, so no two places look alike
        AUDIO::CAudioBuffer audio = MakeInt16( 44100, 2, 30000, SIGNAL_NOISE, 3 );
        Int16* samples = (Int16*) audio.GetDataPtr();
        for ( UInt32 i=0; i<30000 * 2; ++i )
            samples[ i ] = (Int16) ( samples[ i ] / 8 + (Int32) ( 8000.0 * sin( 0.01 * (Float64) i ) ) );
        CORE::CDynamicBuffer file;
        CORE::CValueList options;
        options.Set( "blocksize", "1152" );
        ASSERT_TRUE( Encode( flac, audio, file, options ) );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( flac, file, source ) );
        CRandom random( 2024 );
        CORE::CDynamicBuffer chunk( 1024 * 4, true );
        for ( UInt32 i=0; i<53; ++i )
        {
            UInt32 target = random.Below( 30000 );
            if ( 0 == i ) target = 0;
            if ( 1 == i ) target = 29999;
            if ( 2 == i ) target = 1152;
            if ( 3 == i ) target = 1151;
            ASSERT_TRUE( source->SeekToFrame( target ) );
            const UInt32 expected = 30000 - target < 1024 ? 30000 - target : 1024;
            ASSERT_TRUE( expected == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
            ASSERT_TRUE( 0 == memcmp( chunk.GetConstBufferPtr(), audio.GetDataPtr( target ), expected * 4 ) );
            ASSERT_TRUE( target + expected == source->GetPositionInFrames() );
        }

        // the end is a place too, there is nothing to read there, and nothing further
        ASSERT_TRUE( source->SeekToFrame( 30000 ) );
        ASSERT_TRUE( 0 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
        ASSERT_TRUE( source->IsEndOfStream() );
        ASSERT_FALSE( source->SeekToFrame( 30001 ) );
        ASSERT_TRUE( source->Rewind() );
        ASSERT_FALSE( source->IsEndOfStream() );
        ASSERT_TRUE( 1024 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
        ASSERT_TRUE( 0 == memcmp( chunk.GetConstBufferPtr(), audio.GetDataPtr(), 4096 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: A stream that loops three times has no gap at the seams" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeInt16( 44100, 2, 13579, SIGNAL_TONE );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, audio, file ) );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( flac, file, source ) );

        AUDIO::CAudioBuffer played;
        UInt64 underruns = 1;
        ASSERT_TRUE( PlayLooped( source, 3, played, underruns ) );
        ASSERT_TRUE( 3 * 13579 == played.GetFrameCount() );
        ASSERT_TRUE( 0 == underruns );
        bool isSame = true;
        for ( UInt32 loop=0; loop<3; ++loop )
            isSame = isSame && 0 == memcmp( played.GetDataPtr( loop * 13579 ), audio.GetDataPtr(), audio.GetSizeInBytes() );
        ASSERT_TRUE( isSame );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Channels are in WAVE order: each tone of a 6 channel file is on its channel, and the speaker mask is kept" )
    try
    {
        const AUDIO::CAudioFormat format51( 48000, 6, AUDIO::SAMPLETYPE_INT16 );
        const AUDIO::CAudioBuffer audio = MakeTones( format51, 9000, 500.0, 0.5 );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, audio, file ) );
        AUDIO::CAudioBuffer decoded;
        ASSERT_TRUE( DecodeFile( flac, file, decoded ) );
        ASSERT_TRUE( SameBytes( audio, decoded ) );
        for ( UInt32 c=0; c<6; ++c )
            ASSERT_TRUE( c == ChannelWithTone( decoded, ToneOfChannel( c, 500.0 ), 1000, 4000 ) );
        ASSERT_TRUE( 0x3F == decoded.GetFormat().GetChannelLayout() );

        // a layout that is not the usual one for the count is kept: front, centre, LFE, sides
        const AUDIO::CAudioFormat sides( 48000, 6, AUDIO::SAMPLETYPE_INT16, 0x60F );
        AUDIO::CAudioBuffer sideAudio( sides, 3000 );
        memcpy( sideAudio.GetDataPtr(), audio.GetDataPtr(), sideAudio.GetSizeInBytes() );
        ASSERT_TRUE( Encode( flac, sideAudio, file ) );
        ASSERT_TRUE( DecodeFile( flac, file, decoded ) );
        ASSERT_TRUE( 0x60F == decoded.GetFormat().GetChannelLayout() );
        ASSERT_TRUE( SameBytes( sideAudio, decoded ) );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( flac, file, source ) );
        ASSERT_TRUE( 0x60F == source->GetFormat().GetChannelLayout() );

        // the host puts the layout in the metadata too
        CORE::CValueList metaData;
        CORE::CDynamicBufferAccess input( file );
        ASSERT_TRUE( flac->DecodeMetaData( input, metaData ) );
        ASSERT_TRUE( metaData.GetValueAlways( "channelLayout" ).AsString() == "1551" );
        ASSERT_TRUE( metaData.GetValueAlways( "channels" ).AsString() == "6" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Tags of the file reach the stream and the metadata, with lower case names and the loop points" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeInt16( 44100, 1, 5000, SIGNAL_TONE );
        CORE::CValueList options;
        options.Set( "tags", "TITLE=Test Tune\nArtist=The Band\nLOOPSTART=1000\nLoopLength=2500\nLOOPEND=3500\nNOEQUALSSIGN\n=nameless\nTITLE=Second title\nCOMMENT=caf\xC3\xA9" );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, audio, file, options ) );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( flac, file, source ) );
        CORE::CValueList tags;
        ASSERT_TRUE( source->GetMetaData( tags ) );
        ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Test Tune" );
        ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "The Band" );
        ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "1000" );
        ASSERT_TRUE( tags.GetValueAlways( "looplength" ).AsString() == "2500" );
        ASSERT_TRUE( tags.GetValueAlways( "loopend" ).AsString() == "3500" );
        ASSERT_TRUE( tags.GetValueAlways( "comment" ).AsString() == "caf\xC3\xA9" );
        ASSERT_TRUE( 6 == tags.GetKeyCount() );             // the line without a name and the one without an equals sign are not tags, the first title wins

        // asking does not disturb the stream
        Int16 frames[ 100 ];
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );
        ASSERT_TRUE( source->GetMetaData( tags ) );
        ASSERT_TRUE( 0 == memcmp( frames, audio.GetDataPtr(), 200 ) );

        // the codec reads them from a whole input, and adds what it knows of the format
        CORE::CValueList metaData;
        CORE::CDynamicBufferAccess input( file );
        ASSERT_TRUE( flac->DecodeMetaData( input, metaData ) );
        ASSERT_TRUE( metaData.GetValueAlways( "loopstart" ).AsString() == "1000" );
        ASSERT_TRUE( metaData.GetValueAlways( "sampleRate" ).AsString() == "44100" );
        ASSERT_TRUE( metaData.GetValueAlways( "frames" ).AsString() == "5000" );

        // through a stream (the way music plays) the tags are available too
        AUDIO::CAudioStream stream( source, 200, 1, false );
        CORE::CValueList streamTags;
        ASSERT_TRUE( stream.GetMetaData( streamTags ) );
        ASSERT_TRUE( streamTags.GetValueAlways( "looplength" ).AsString() == "2500" );

        // no tags: no metadata from the stream
        CORE::CDynamicBuffer plain;
        ASSERT_TRUE( Encode( flac, audio, plain ) );
        AUDIO::TAudioSourcePtr plainSource;
        ASSERT_TRUE( OpenSource( flac, plain, plainSource ) );
        CORE::CValueList none;
        ASSERT_FALSE( plainSource->GetMetaData( none ) );
        ASSERT_TRUE( 0 == none.GetKeyCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: The probe knows FLAC data, also behind an ID3 tag, and nothing else" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeInt16( 44100, 2, 2000, SIGNAL_TONE );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, audio, file ) );

        CORE::CDynamicBufferAccess input( file );
        UInt32 confidence = 0;
        ASSERT_TRUE( flac->Probe( input, confidence ) );
        ASSERT_TRUE( 100 == confidence );
        ASSERT_TRUE( 0 == input.Tell() );

        TCodecPtr found;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( input, found, &confidence ) );
        ASSERT_TRUE( found->GetType() == "flac" && 100 == confidence );

        // an ID3v2 tag of 20 bytes in front
        CORE::CDynamicBuffer tagged;
        const UInt8 id3[ 10 ] = { 'I', 'D', '3', 3, 0, 0, 0, 0, 0, 20 };
        tagged.Append( id3, 10 );
        UInt8 zeros[ 20 ];
        memset( zeros, 0, sizeof( zeros ) );
        tagged.Append( zeros, 20 );
        tagged.Append( file.GetConstBufferPtr(), file.GetDataSize() );
        CORE::CDynamicBufferAccess taggedInput( tagged );
        ASSERT_TRUE( flac->Probe( taggedInput, confidence ) );
        ASSERT_TRUE( 90 == confidence );
        // and it decodes: dr_flac skips the tag
        AUDIO::CAudioBuffer decoded;
        ASSERT_TRUE( DecodeFile( flac, tagged, decoded ) );
        ASSERT_TRUE( SameBytes( audio, decoded ) );

        // nothing that is FLAC
        CRandom random( 8 );
        CORE::CDynamicBuffer junk( 300, true );
        for ( UInt32 i=0; i<300; ++i )
            ( (UInt8*) junk.GetBufferPtr() )[ i ] = (UInt8) random.Next();
        CORE::CDynamicBufferAccess junkInput( junk );
        ASSERT_TRUE( flac->Probe( junkInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );
        ASSERT_FALSE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( junkInput, found ) );
        CORE::CDynamicBuffer tiny( "fL", 2, true );
        CORE::CDynamicBufferAccess tinyInput( tiny );
        ASSERT_TRUE( flac->Probe( tinyInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );
        AUDIO::TAudioSourcePtr none;
        ASSERT_FALSE( OpenSource( flac, junk, none ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Truncated and corrupted files never crash or hang the decoder" )
    try
    {
        AUDIO::CAudioBuffer audio = MakeInt16( 22050, 2, 700, SIGNAL_NOISE, 21 );
        Int16* samples = (Int16*) audio.GetDataPtr();
        for ( UInt32 i=0; i<700 * 2; ++i )
            samples[ i ] = (Int16) ( samples[ i ] / 16 + (Int32) ( 5000.0 * sin( 0.02 * (Float64) i ) ) );
        CORE::CValueList options;
        options.Set( "blocksize", "128" );
        options.Set( "tags", "TITLE=Sweep\nLOOPSTART=5" );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, audio, file, options ) );
        ASSERT_TRUE( SweepDamage( flac, file, 1, 1000, 100000000, 17 ) == 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Two streams of the plugin decode at the same time, per source" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeInt16( 44100, 2, 90000, SIGNAL_NOISE, 13 );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( Encode( flac, audio, file ) );
        Int32 maxOverlap = 0;
        ASSERT_TRUE( DecodeTwoAtOnce( flac, file, audio, maxOverlap ) );
        ASSERT_TRUE( 2 == maxOverlap );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Writes FLAC files of generated signals into GUCEF_TEST_AUDIO_WRITE_SIGNALS (opt in)" )
    {
        const CORE::CString directory( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_WRITE_SIGNALS" ).c_str() );
        GUCEF_TESTFW_SKIP_IF( directory.IsNULLOrEmpty(), "GUCEF_TEST_AUDIO_WRITE_SIGNALS is not set" );
        try
        {
            // in the layout the conformance test reads: the files that must decode with the MD5 of their STREAMINFO
            const CORE::CString target = CORE::CombinePath( CORE::CombinePath( directory, "flac" ), "subset" );
            ASSERT_TRUE( CORE::CreatePathDirectories( CORE::CombinePath( target, "file" ) ) );
            const AUDIO::CAudioBuffer files[] = {
                MakeInt16( 44100, 2, 44100, SIGNAL_TONE ), MakeInt16( 8000, 1, 8000, SIGNAL_NOISE ),
                MakeInt16( 48000, 6, 9600, SIGNAL_STEPS ), MakeInt16( 16000, 1, 3000, SIGNAL_EXTREMES ) };
            const char* names[] = { "generated-stereo-tone.flac", "generated-mono-noise.flac", "generated-six-channel-steps.flac", "generated-extremes.flac" };
            for ( UInt32 i=0; i<4; ++i )
            {
                CORE::CDynamicBuffer file;
                ASSERT_TRUE( Encode( flac, files[ i ], file ) );
                ASSERT_TRUE( file.WriteContentToFile( CORE::CombinePath( target, names[ i ] ), 0, true ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: The FLAC conformance files in GUCEF_TEST_AUDIO_ASSETS decode with the MD5 of their STREAMINFO (opt in)" )
    {
        const CORE::CString assets = GetAssetsDirectory();
        GUCEF_TESTFW_SKIP_IF( assets.IsNULLOrEmpty(), "GUCEF_TEST_AUDIO_ASSETS is not set" );
        const CORE::CString flacDirectory = CORE::CombinePath( assets, "flac" );
        GUCEF_TESTFW_SKIP_IF( !CORE::DirExists( flacDirectory ), "there is no flac directory in GUCEF_TEST_AUDIO_ASSETS" );
        try
        {
            // subset: has to decode, and match
            const GUCEF::vector< CORE::CString > subset = ListFlacFiles( CORE::CombinePath( flacDirectory, "subset" ) );
            for ( size_t i=0; i<subset.size(); ++i )
            {
                CORE::CDynamicBuffer file;
                ASSERT_TRUE( file.LoadContentFromFile( subset[ i ] ) );
                AUDIO::CAudioBuffer decoded;
                ASSERT_TRUE( DecodeFile( flac, file, decoded ) );
                UInt8 stored[ 16 ];
                UInt8 computed[ 16 ];
                UInt32 bits = 0;
                if ( GetStreamInfoMd5( file, stored, bits ) )
                {
                    Md5OfDecoded( decoded, bits, computed );
                    ASSERT_TRUE( 0 == memcmp( stored, computed, 16 ) );
                }
            }

            // uncommon: decode or are turned down without a crash
            const GUCEF::vector< CORE::CString > uncommon = ListFlacFiles( CORE::CombinePath( flacDirectory, "uncommon" ) );
            for ( size_t i=0; i<uncommon.size(); ++i )
            {
                CORE::CDynamicBuffer file;
                if ( file.LoadContentFromFile( uncommon[ i ] ) )
                    ASSERT_TRUE( UseDamagedFile( flac, file, 4000000000ULL, 30000, 1 ) );
            }

            // faulty: never a crash or a hang
            const GUCEF::vector< CORE::CString > faulty = ListFlacFiles( CORE::CombinePath( flacDirectory, "faulty" ) );
            for ( size_t i=0; i<faulty.size(); ++i )
            {
                CORE::CDynamicBuffer file;
                if ( file.LoadContentFromFile( faulty[ i ] ) )
                    ASSERT_TRUE( UseDamagedFile( flac, file, 4000000000ULL, 30000, 2 ) );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FLAC conformance files: " << subset.size() << " subset, " << uncommon.size() << " uncommon, " << faulty.size() << " faulty" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecFlac: Unloading the plugin takes the type away" )
    try
    {
        flac = TCodecPtr();
        ASSERT_TRUE( UnloadPlugin( PluginGroup, "flac" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
