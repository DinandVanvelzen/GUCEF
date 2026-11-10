/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioCodecs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Bytes in memory that can be read as a file */
class CMemoryFile
{
    public:

    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access;

    CMemoryFile( const void* data, UInt32 size ) : buffer(), access( buffer )
    {
        if ( size > 0 )
            buffer.Append( data, size );
        access.Setpos( 0 );
    }

    explicit CMemoryFile( const GUCEF::vector< UInt8 >& bytes ) : buffer(), access( buffer )
    {
        if ( !bytes.empty() )
            buffer.Append( &bytes[ 0 ], (UInt32) bytes.size() );
        access.Setpos( 0 );
    }

    CMemoryFile( void ) : buffer(), access( buffer ) {}
};

void PutLE16( GUCEF::vector< UInt8 >& out, UInt32 v ) { out.push_back( (UInt8) ( v & 0xFF ) ); out.push_back( (UInt8) ( ( v >> 8 ) & 0xFF ) ); }
void PutLE32( GUCEF::vector< UInt8 >& out, UInt32 v ) { PutLE16( out, v & 0xFFFF ); PutLE16( out, v >> 16 ); }
void PutTag( GUCEF::vector< UInt8 >& out, const char* tag ) { for ( int i=0; i<4; ++i ) out.push_back( (UInt8) tag[ i ] ); }

/** A canonical 44 byte header WAVE file with the given samples */
GUCEF::vector< UInt8 >
MakeWav( UInt32 tag, UInt32 channels, UInt32 rate, UInt32 bits, const GUCEF::vector< UInt8 >& samples, UInt32 declaredDataSize = 0xFFFFFFFE )
{
    GUCEF::vector< UInt8 > f;
    PutTag( f, "RIFF" ); PutLE32( f, 36 + (UInt32) samples.size() ); PutTag( f, "WAVE" );
    PutTag( f, "fmt " ); PutLE32( f, 16 ); PutLE16( f, tag ); PutLE16( f, channels ); PutLE32( f, rate );
    PutLE32( f, rate * channels * bits / 8 ); PutLE16( f, channels * bits / 8 ); PutLE16( f, bits );
    PutTag( f, "data" ); PutLE32( f, 0xFFFFFFFE == declaredDataSize ? (UInt32) samples.size() : declaredDataSize );
    f.insert( f.end(), samples.begin(), samples.end() );
    return f;
}

bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-5f )
{
    return fabsf( a - b ) <= tolerance;
}

AUDIO::CAudioBuffer
MakeTestAudio( const AUDIO::CAudioFormat& format, UInt32 frames )
{
    AUDIO::CAudioBuffer audio( format, frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
            audio.SetSampleFromFloat( f, c, (Float32) sin( (f + 1) * 0.05 * ( c + 1 ) ) * 0.8f );
    return audio;
}

bool
SameBytes( const AUDIO::CAudioBuffer& a, const AUDIO::CAudioBuffer& b )
{
    return a.GetFormat() == b.GetFormat() && a.GetFrameCount() == b.GetFrameCount() && 0 == memcmp( a.GetDataPtr(), b.GetDataPtr(), a.GetSizeInBytes() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformClipTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioClip: Sources of one clip play independently and the clip is shared, not copied" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeTestAudio( AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ), 1000 );
        AUDIO::CAudioClip clip( audio, "beep" );
        ASSERT_TRUE( clip.GetName() == "beep" );
        ASSERT_TRUE( 1000 == clip.GetFrameCount() );
        ASSERT_TRUE( Near( 1000.0f / 22050.0f, (Float32) clip.GetDurationInSeconds() ) );
        ASSERT_FALSE( clip.IsEmpty() );

        AUDIO::TAudioSourcePtr a = clip.CreateSource(), b = clip.CreateSource();
        ASSERT_FALSE( a.IsNULL() );
        ASSERT_TRUE( a->GetFormat() == audio.GetFormat() );
        ASSERT_TRUE( 1000 == a->GetLengthInFrames() );
        ASSERT_TRUE( a->IsSeekable() );

        Int16 first[ 300 ], second[ 300 ];
        ASSERT_TRUE( 300 == a->ReadFrames( first, 300 ) );
        ASSERT_TRUE( 300 == a->GetPositionInFrames() );
        ASSERT_TRUE( 300 == b->ReadFrames( second, 300 ) );
        ASSERT_TRUE( 0 == memcmp( first, second, sizeof( first ) ) );                           // same audio from the start
        ASSERT_TRUE( 0 == memcmp( first, audio.GetDataPtr(), sizeof( first ) ) );
        Int16 rest[ 1000 ];
        ASSERT_TRUE( 700 == a->ReadFrames( rest, 1000 ) );                                      // only what is left
        ASSERT_TRUE( a->IsEndOfStream() );
        ASSERT_TRUE( 0 == a->ReadFrames( rest, 10 ) );
        ASSERT_FALSE( b->IsEndOfStream() );                                                     // the other one is not done

        ASSERT_TRUE( a->Rewind() );
        ASSERT_FALSE( a->IsEndOfStream() );
        ASSERT_TRUE( 0 == a->GetPositionInFrames() );
        ASSERT_TRUE( a->SeekToFrame( 990 ) );
        ASSERT_TRUE( 10 == a->ReadFrames( first, 300 ) );
        ASSERT_FALSE( a->SeekToFrame( 1001 ) );                                                 // beyond the end
        ASSERT_TRUE( a->SeekToFrame( 1000 ) && a->IsEndOfStream() );                            // the end itself is fine

        // the clip shares the buffer it was given and the sources keep it alive
        AUDIO::TAudioBufferPtr shared( GUCEF_NEW AUDIO::CAudioBuffer( audio ) );
        AUDIO::TAudioSourcePtr outlive;
        {
            AUDIO::CAudioClip sharing( shared );
            ASSERT_TRUE( sharing.GetSharedBuffer().GetPointerAlways() == shared.GetPointerAlways() );
            outlive = sharing.CreateSource();
        }
        shared = AUDIO::TAudioBufferPtr();
        ASSERT_TRUE( 300 == outlive->ReadFrames( first, 300 ) );
        ASSERT_TRUE( 0 == memcmp( first, audio.GetDataPtr(), sizeof( first ) ) );

        AUDIO::CAudioClip empty;
        ASSERT_TRUE( empty.IsEmpty() );
        AUDIO::TAudioSourcePtr none = empty.CreateSource();
        ASSERT_TRUE( none->IsEndOfStream() || 0 == none->ReadFrames( first, 10 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioCodecRegistry: The built in codecs are there and found by name, extension and file name" )
    try
    {
        AUDIO::CAudioCodecRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry();
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr codec;
        ASSERT_TRUE( registry.TryFindCodec( "wav", codec ) );
        ASSERT_TRUE( codec->GetType() == "wav" );
        ASSERT_TRUE( codec->GetFamilyName() == "AudioCodec" );
        ASSERT_TRUE( codec->CanEncode() );
        ASSERT_TRUE( registry.TryFindCodec( "WAV", codec ) );               // case does not matter
        ASSERT_TRUE( registry.TryFindCodec( ".Wav", codec ) );              // nor does a leading dot
        ASSERT_TRUE( registry.TryFindCodec( "raw", codec ) );
        ASSERT_TRUE( registry.TryFindCodecForFilename( "sounds/boom.WAV", codec ) && codec->GetType() == "wav" );
        ASSERT_TRUE( registry.TryFindCodecForFilename( "C:\\sounds\\boom.wav", codec ) );
        ASSERT_FALSE( registry.TryFindCodecForFilename( "sounds/boom", codec ) );
        ASSERT_FALSE( registry.TryFindCodecForFilename( "sounds.d/boom", codec ) );     // a dot in a folder is not an extension
        ASSERT_FALSE( registry.TryFindCodecForFilename( "boom.", codec ) );
        ASSERT_FALSE( registry.TryFindCodec( "doesnotexist", codec ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCodecRegistry: More codecs can be registered and codecs can be cloned" )
    try
    {
        AUDIO::CAudioCodecRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry();
        ASSERT_TRUE( registry.TryRegister( "testtone", AUDIO::TAudioCodecPtr( GUCEF_NEW AUDIO::CRawAudioCodec() ) ) );
        ASSERT_FALSE( registry.TryRegister( "testtone", AUDIO::TAudioCodecPtr( GUCEF_NEW AUDIO::CRawAudioCodec() ) ) );   // the name is taken
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr codec;
        ASSERT_TRUE( registry.TryFindCodec( "TestTone", codec ) );
        registry.Unregister( "testtone" );
        ASSERT_FALSE( registry.TryFindCodec( "testtone", codec ) );

        AUDIO::CWavAudioCodec wav;
        CORE::CICloneable* clone = wav.Clone();
        ASSERT_TRUE( GUCEF_NULL != clone && GUCEF_NULL != CORE::CICloneable::CloneableAs< AUDIO::CWavAudioCodec >( clone ) );
        GUCEF_DELETE clone;
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformWavTests( void )
{GUCEF_TRACE;

    AUDIO::CWavAudioCodec wav;

    GUCEF_TESTFW_TESTCASE( "WavCodec: Reads a plain 16 bit stereo file byte for byte" )
    try
    {
        GUCEF::vector< UInt8 > samples;
        const Int16 values[ 6 ] = { 0, 1000, -1000, 32767, -32768, 12345 };
        for ( UInt32 i=0; i<6; ++i )
            PutLE16( samples, (UInt16) values[ i ] );
        CMemoryFile file( MakeWav( 1, 2, 44100, 16, samples ) );

        AUDIO::CWavAudioCodec::SWavInfo info;
        ASSERT_TRUE( AUDIO::CWavAudioCodec::ParseHeader( file.access, info ) );
        ASSERT_TRUE( info.format == AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( 44 == info.dataOffset );
        ASSERT_TRUE( 3 == info.frameCount );

        file.access.Setpos( 0 );
        AUDIO::CAudioBuffer audio;
        ASSERT_TRUE( wav.DecodeAll( file.access, audio ) );
        ASSERT_TRUE( 3 == audio.GetFrameCount() );
        ASSERT_TRUE( 0 == memcmp( audio.GetDataPtr(), values, 12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: Every sample type and channel layout survives writing and reading" )
    try
    {
        const AUDIO::TSampleType types[ 6 ] = { AUDIO::SAMPLETYPE_UINT8, AUDIO::SAMPLETYPE_INT16, AUDIO::SAMPLETYPE_INT24, AUDIO::SAMPLETYPE_INT32, AUDIO::SAMPLETYPE_FLOAT32, AUDIO::SAMPLETYPE_FLOAT64 };
        const UInt32 channelCounts[ 5 ] = { 1, 2, 3, 6, 8 };
        for ( UInt32 t=0; t<6; ++t )
            for ( UInt32 c=0; c<5; ++c )
            {
                const AUDIO::CAudioFormat format( 48000, channelCounts[ c ], types[ t ] );
                const AUDIO::CAudioBuffer original = MakeTestAudio( format, 257 );
                CMemoryFile file;
                ASSERT_TRUE( wav.Encode( original, file.access ) );
                file.access.Setpos( 0 );
                AUDIO::CAudioBuffer decoded;
                ASSERT_TRUE( wav.DecodeAll( file.access, decoded ) );
                ASSERT_TRUE( SameBytes( original, decoded ) );
            }

        // a layout that is not the usual one for its channel count is kept
        AUDIO::CAudioFormat odd( 44100, 0, AUDIO::SAMPLETYPE_INT16, AUDIO::AUDIOSPEAKER_FRONT_CENTER | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY );
        const AUDIO::CAudioBuffer oddAudio = MakeTestAudio( odd, 50 );
        CMemoryFile oddFile;
        ASSERT_TRUE( wav.Encode( oddAudio, oddFile.access ) );
        oddFile.access.Setpos( 0 );
        AUDIO::CAudioBuffer oddDecoded;
        ASSERT_TRUE( wav.DecodeAll( oddFile.access, oddDecoded ) );
        ASSERT_TRUE( oddDecoded.GetFormat().GetChannelLayout() == ( AUDIO::AUDIOSPEAKER_FRONT_CENTER | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY ) );
        ASSERT_TRUE( SameBytes( oddAudio, oddDecoded ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: Plain files get the plain header, extensible is used when needed" )
    try
    {
        CMemoryFile plain;
        ASSERT_TRUE( wav.Encode( MakeTestAudio( AUDIO::CAudioFormat( 8000, 2, AUDIO::SAMPLETYPE_INT16 ), 10 ), plain.access ) );
        ASSERT_TRUE( 44 + 40 == plain.buffer.GetDataSize() );
        const UInt8* bytes = static_cast< const UInt8* >( plain.buffer.GetConstBufferPtr() );
        ASSERT_TRUE( 0 == memcmp( bytes, "RIFF", 4 ) && 0 == memcmp( bytes + 8, "WAVEfmt ", 8 ) );
        ASSERT_TRUE( 16 == bytes[ 16 ] && 1 == bytes[ 20 ] );               // fmt size 16, format tag PCM

        CMemoryFile wide;
        ASSERT_TRUE( wav.Encode( MakeTestAudio( AUDIO::CAudioFormat( 8000, 6, AUDIO::SAMPLETYPE_INT16 ), 10 ), wide.access ) );
        const UInt8* wideBytes = static_cast< const UInt8* >( wide.buffer.GetConstBufferPtr() );
        ASSERT_TRUE( 40 == wideBytes[ 16 ] && 0xFE == wideBytes[ 20 ] && 0xFF == wideBytes[ 21 ] );

        CMemoryFile floats;
        ASSERT_TRUE( wav.Encode( MakeTestAudio( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 10 ), floats.access ) );
        ASSERT_TRUE( 3 == static_cast< const UInt8* >( floats.buffer.GetConstBufferPtr() )[ 20 ] ); // IEEE float tag
        ASSERT_TRUE( 18 == static_cast< const UInt8* >( floats.buffer.GetConstBufferPtr() )[ 16 ] ); // fmt with the size field for extra data
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: Other chunks are skipped, odd sized chunks are padded, unknown data sizes read to the end" )
    try
    {
        GUCEF::vector< UInt8 > samples;
        for ( UInt32 i=0; i<10; ++i )
            PutLE16( samples, i * 100 );

        // LIST chunk of odd size (with its pad byte) between fmt and data
        GUCEF::vector< UInt8 > f;
        PutTag( f, "RIFF" ); PutLE32( f, 0 ); PutTag( f, "WAVE" );
        PutTag( f, "fmt " ); PutLE32( f, 16 ); PutLE16( f, 1 ); PutLE16( f, 1 ); PutLE32( f, 8000 ); PutLE32( f, 16000 ); PutLE16( f, 2 ); PutLE16( f, 16 );
        PutTag( f, "LIST" ); PutLE32( f, 5 ); for ( UInt32 i=0; i<5; ++i ) f.push_back( 'x' ); f.push_back( 0 );
        PutTag( f, "data" ); PutLE32( f, 0xFFFFFFFF );
        f.insert( f.end(), samples.begin(), samples.end() );
        CMemoryFile file( f );
        AUDIO::CAudioBuffer audio;
        ASSERT_TRUE( wav.DecodeAll( file.access, audio ) );
        ASSERT_TRUE( 10 == audio.GetFrameCount() );
        ASSERT_TRUE( 0 == memcmp( audio.GetDataPtr(), &samples[ 0 ], 20 ) );

        // fewer samples in the file than the header promises: what is there is what you get
        GUCEF::vector< UInt8 > truncated = MakeWav( 1, 1, 8000, 16, samples, 2000 );
        CMemoryFile cut( truncated );
        AUDIO::CAudioBuffer partial;
        ASSERT_TRUE( wav.DecodeAll( cut.access, partial ) );
        ASSERT_TRUE( 10 == partial.GetFrameCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: Files it cannot read are refused, not misread" )
    try
    {
        GUCEF::vector< UInt8 > samples( 8, 0 );
        AUDIO::TAudioSourcePtr source;

        GUCEF::vector< UInt8 > bad = MakeWav( 1, 1, 8000, 16, samples );
        bad[ 0 ] = 'X';                                                         // not RIFF
        CMemoryFile notRiff( bad );
        ASSERT_FALSE( wav.OpenSource( notRiff.access, source ) );

        GUCEF::vector< UInt8 > notWave = MakeWav( 1, 1, 8000, 16, samples );
        notWave[ 8 ] = 'X';
        CMemoryFile notWaveFile( notWave );
        ASSERT_FALSE( wav.OpenSource( notWaveFile.access, source ) );

        CMemoryFile adpcm( MakeWav( 2, 1, 8000, 4, samples ) );                  // ADPCM
        ASSERT_FALSE( wav.OpenSource( adpcm.access, source ) );

        CMemoryFile oddBits( MakeWav( 1, 1, 8000, 12, samples ) );
        ASSERT_FALSE( wav.OpenSource( oddBits.access, source ) );

        CMemoryFile noChannels( MakeWav( 1, 0, 8000, 16, samples ) );
        ASSERT_FALSE( wav.OpenSource( noChannels.access, source ) );

        GUCEF::vector< UInt8 > wrongAlign = MakeWav( 1, 2, 8000, 16, samples );
        wrongAlign[ 32 ] = 7;                                                    // block align
        CMemoryFile alignFile( wrongAlign );
        ASSERT_FALSE( wav.OpenSource( alignFile.access, source ) );

        GUCEF::vector< UInt8 > noData = MakeWav( 1, 1, 8000, 16, samples );
        noData.resize( 36 );                                                     // header and fmt only
        CMemoryFile noDataFile( noData );
        ASSERT_FALSE( wav.OpenSource( noDataFile.access, source ) );

        GUCEF::vector< UInt8 > cutShort = MakeWav( 1, 1, 8000, 16, samples );
        cutShort.resize( 30 );                                                   // cut in the middle of fmt
        CMemoryFile cutFile( cutShort );
        ASSERT_FALSE( wav.OpenSource( cutFile.access, source ) );

        CMemoryFile nothing;
        ASSERT_FALSE( wav.OpenSource( nothing.access, source ) );

        // every cut of a good file ends in a refusal or a short file, never in a crash
        GUCEF::vector< UInt8 > good = MakeWav( 1, 2, 8000, 16, samples );
        for ( UInt32 n=0; n<good.size(); ++n )
        {
            CMemoryFile partialFile( &good[ 0 ], n );
            AUDIO::CAudioBuffer ignored;
            wav.DecodeAll( partialFile.access, ignored );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: Streaming in small pieces gives the same audio, with position, length, seek and rewind" )
    try
    {
        const AUDIO::CAudioBuffer original = MakeTestAudio( AUDIO::CAudioFormat( 32000, 2, AUDIO::SAMPLETYPE_INT16 ), 5000 );
        CMemoryFile file;
        ASSERT_TRUE( wav.Encode( original, file.access ) );
        file.access.Setpos( 0 );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( wav.OpenSource( file.access, source ) );
        ASSERT_TRUE( 5000 == source->GetLengthInFrames() );
        ASSERT_TRUE( source->IsSeekable() );
        ASSERT_FALSE( source->IsEndOfStream() );

        AUDIO::CAudioBuffer streamed( original.GetFormat(), 0 );
        UInt32 piece = 1;
        while ( !source->IsEndOfStream() )
        {
            AUDIO::CAudioBuffer chunk( original.GetFormat(), piece );
            const UInt32 got = source->ReadFrames( chunk.GetDataPtr(), piece );
            chunk.Resize( got );
            ASSERT_TRUE( streamed.Append( chunk ) );
            ASSERT_TRUE( streamed.GetFrameCount() == source->GetPositionInFrames() );
            piece = ( piece * 3 ) % 700 + 1;
        }
        ASSERT_TRUE( SameBytes( original, streamed ) );
        ASSERT_TRUE( 5000 == source->GetPositionInFrames() );

        ASSERT_TRUE( source->SeekToFrame( 4990 ) );
        ASSERT_FALSE( source->IsEndOfStream() );
        Int16 tail[ 40 ];
        ASSERT_TRUE( 10 == source->ReadFrames( tail, 20 ) );
        ASSERT_TRUE( 0 == memcmp( tail, original.GetDataPtr( 4990 ), 40 ) );
        ASSERT_TRUE( source->IsEndOfStream() );
        ASSERT_TRUE( source->Rewind() );
        ASSERT_TRUE( 20 == source->ReadFrames( tail, 20 ) );
        ASSERT_TRUE( 0 == memcmp( tail, original.GetDataPtr(), 80 ) );
        ASSERT_FALSE( source->SeekToFrame( 5001 ) );
        ASSERT_TRUE( 20 == source->GetPositionInFrames() );                      // a refused seek leaves the position
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: Metadata, clips from the registry and the generic decode" )
    try
    {
        const AUDIO::CAudioBuffer original = MakeTestAudio( AUDIO::CAudioFormat( 11025, 1, AUDIO::SAMPLETYPE_UINT8 ), 22050 );
        CMemoryFile file;
        ASSERT_TRUE( wav.Encode( original, file.access ) );

        file.access.Setpos( 0 );
        CORE::CValueList meta;
        ASSERT_TRUE( wav.DecodeMetaData( file.access, meta ) );
        ASSERT_TRUE( 11025 == meta.GetValueAlways( "sampleRate" ).AsUInt32() );
        ASSERT_TRUE( 1 == meta.GetValueAlways( "channels" ).AsUInt32() );
        ASSERT_TRUE( meta.GetValueAlways( "sampleType" ).AsString() == "uint8" );
        ASSERT_TRUE( 22050 == meta.GetValueAlways( "frames" ).AsUInt64() );
        ASSERT_TRUE( 2000 == meta.GetValueAlways( "durationInMs" ).AsUInt64() );

        file.access.Setpos( 0 );
        AUDIO::TAudioClipPtr clip;
        ASSERT_TRUE( AUDIO::CAudioClip::Decode( "WAV", file.access, clip, CORE::CValueList(), "from file" ) );
        ASSERT_TRUE( clip->GetName() == "from file" && 22050 == clip->GetFrameCount() );
        ASSERT_TRUE( SameBytes( original, clip->GetBuffer() ) );

        file.access.Setpos( 0 );
        AUDIO::TAudioClipPtr unknown;
        ASSERT_FALSE( AUDIO::CAudioClip::Decode( "nosuchcodec", file.access, unknown ) );

        // the generic codec interface writes the plain samples
        file.access.Setpos( 0 );
        CMemoryFile plainOut;
        ASSERT_TRUE( wav.Decode( file.access, plainOut.access ) );
        ASSERT_TRUE( 22050 == plainOut.buffer.GetDataSize() );
        ASSERT_TRUE( 0 == memcmp( plainOut.buffer.GetConstBufferPtr(), original.GetDataPtr(), 22050 ) );
        CMemoryFile sink;
        ASSERT_FALSE( wav.Encode( file.access, sink.access ) );                  // there is no plain byte form to start from

        // not encodable audio is refused
        CMemoryFile out;
        ASSERT_FALSE( wav.Encode( AUDIO::CAudioBuffer(), out.access ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WavCodec: A source that was given the input keeps it alive" )
    try
    {
        const AUDIO::CAudioBuffer original = MakeTestAudio( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 300 );
        CORE::CDynamicBuffer* bytes = GUCEF_NEW CORE::CDynamicBuffer();
        CORE::CDynamicBufferAccess* access = GUCEF_NEW CORE::CDynamicBufferAccess( bytes, true );
        ASSERT_TRUE( wav.Encode( original, *access ) );
        access->Setpos( 0 );

        AUDIO::TAudioSourcePtr source;
        {
            CORE::CIOAccess::IOAccessPtr shared( access );
            ASSERT_TRUE( wav.OpenSource( shared, source ) );
        }                                                                       // the only other reference is gone
        Int16 samples[ 300 ];
        ASSERT_TRUE( 300 == source->ReadFrames( samples, 300 ) );
        ASSERT_TRUE( 0 == memcmp( samples, original.GetDataPtr(), 600 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRawTests( void )
{GUCEF_TRACE;

    AUDIO::CRawAudioCodec raw;

    GUCEF_TESTFW_TESTCASE( "RawCodec: Unsigned 8 bit mono sound data at 22000 Hz, the form of old game sound" )
    try
    {
        const UInt8 data[ 6 ] = { 128, 255, 0, 192, 64, 128 };
        CMemoryFile file( data, 6 );

        CORE::CValueList options;
        options.Set( "sampleRate", "22000" );
        options.Set( "channels", "1" );
        options.Set( "sampleType", "uint8" );

        AUDIO::CAudioBuffer audio;
        ASSERT_TRUE( raw.DecodeAll( file.access, audio, options ) );
        ASSERT_TRUE( audio.GetFormat() == AUDIO::CAudioFormat( 22000, 1, AUDIO::SAMPLETYPE_UINT8 ) );
        ASSERT_TRUE( 6 == audio.GetFrameCount() );
        ASSERT_TRUE( 0 == memcmp( audio.GetDataPtr(), data, 6 ) );
        ASSERT_TRUE( Near( 0.0f, audio.GetSampleAsFloat( 0, 0 ) ) && Near( -1.0f, audio.GetSampleAsFloat( 2, 0 ) ) && Near( 0.5f, audio.GetSampleAsFloat( 3, 0 ) ) );

        // turned into what the sound device wants
        ASSERT_TRUE( audio.ConvertFormatTo( AUDIO::CAudioFormat( 44000, 2, AUDIO::SAMPLETYPE_INT16 ) ) );
        ASSERT_TRUE( 12 == audio.GetFrameCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RawCodec: Byte order, header skipping, frame limit and defaults" )
    try
    {
        // 16 bit big endian stereo, 4 header bytes in front, only 2 frames wanted
        const UInt8 data[ 4 + 12 ] = { 9, 9, 9, 9,  0x40, 0x00, 0xC0, 0x00,  0x7F, 0xFF, 0x80, 0x00,  0x11, 0x11, 0x22, 0x22 };
        CMemoryFile file( data, sizeof( data ) );
        CORE::CValueList options;
        options.Set( "sampleRate", "8000" );
        options.Set( "channels", "2" );
        options.Set( "sampleType", "int16" );
        options.Set( "bigEndian", "true" );
        options.Set( "headerBytes", "4" );
        options.Set( "frames", "2" );
        AUDIO::CAudioBuffer audio;
        ASSERT_TRUE( raw.DecodeAll( file.access, audio, options ) );
        ASSERT_TRUE( 2 == audio.GetFrameCount() );
        ASSERT_TRUE( Near( 0.5f, audio.GetSampleAsFloat( 0, 0 ), 1e-4f ) && Near( -0.5f, audio.GetSampleAsFloat( 0, 1 ), 1e-4f ) );
        ASSERT_TRUE( Near( 1.0f, audio.GetSampleAsFloat( 1, 0 ), 1e-3f ) && Near( -1.0f, audio.GetSampleAsFloat( 1, 1 ), 1e-4f ) );

        // defaults: 44100 Hz mono int16
        CMemoryFile defaults( data + 4, 4 );
        AUDIO::CAudioBuffer plain;
        ASSERT_TRUE( raw.DecodeAll( defaults.access, plain ) );
        ASSERT_TRUE( plain.GetFormat() == AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_INT16 ) && 2 == plain.GetFrameCount() );

        // nonsense options are refused
        CORE::CValueList bad;
        bad.Set( "sampleType", "int17" );
        CMemoryFile again( data, sizeof( data ) );
        AUDIO::TAudioSourcePtr source;
        ASSERT_FALSE( raw.OpenSource( again.access, source, bad ) );
        CORE::CValueList noChannels;
        noChannels.Set( "channels", "0" );
        ASSERT_FALSE( raw.OpenSource( again.access, source, noChannels ) );
        CORE::CValueList meta;
        ASSERT_FALSE( raw.DecodeMetaData( again.access, meta ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RawCodec: Writing and reading back, in both byte orders, and seeking" )
    try
    {
        const AUDIO::CAudioBuffer original = MakeTestAudio( AUDIO::CAudioFormat( 16000, 2, AUDIO::SAMPLETYPE_INT24 ), 400 );
        for ( UInt32 order=0; order<2; ++order )
        {
            CORE::CValueList options;
            options.Set( "sampleRate", "16000" );
            options.Set( "channels", "2" );
            options.Set( "sampleType", "int24" );
            options.Set( "bigEndian", order ? "true" : "false" );

            CMemoryFile file;
            ASSERT_TRUE( raw.Encode( original, file.access, options ) );
            ASSERT_TRUE( 400 * 6 == file.buffer.GetDataSize() );
            file.access.Setpos( 0 );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( raw.DecodeAll( file.access, decoded, options ) );
            ASSERT_TRUE( SameBytes( original, decoded ) );
        }

        // the two byte orders really differ on the wire
        CORE::CValueList little, big;
        little.Set( "bigEndian", "false" );
        big.Set( "bigEndian", "true" );
        CMemoryFile a, b;
        const AUDIO::CAudioBuffer pcm = MakeTestAudio( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 10 );
        ASSERT_TRUE( raw.Encode( pcm, a.access, little ) && raw.Encode( pcm, b.access, big ) );
        const UInt8* la = static_cast< const UInt8* >( a.buffer.GetConstBufferPtr() );
        const UInt8* lb = static_cast< const UInt8* >( b.buffer.GetConstBufferPtr() );
        ASSERT_TRUE( la[ 2 ] == lb[ 3 ] && la[ 3 ] == lb[ 2 ] );

        CORE::CValueList options;
        options.Set( "sampleRate", "8000" );
        CMemoryFile seekFile( a.buffer.GetConstBufferPtr(), a.buffer.GetDataSize() );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( raw.OpenSource( seekFile.access, source, options ) );
        ASSERT_TRUE( source->IsSeekable() );
        ASSERT_TRUE( source->SeekToFrame( 7 ) );
        Int16 three[ 3 ];
        ASSERT_TRUE( 3 == source->ReadFrames( three, 10 ) );
        ASSERT_TRUE( 0 == memcmp( three, pcm.GetDataPtr( 7 ), 6 ) );
        ASSERT_TRUE( source->IsEndOfStream() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioCodecTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioCodec TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Codecs" );

    PerformClipTests();
    PerformRegistryTests();
    PerformWavTests();
    PerformRawTests();
}

/*-------------------------------------------------------------------------*/
