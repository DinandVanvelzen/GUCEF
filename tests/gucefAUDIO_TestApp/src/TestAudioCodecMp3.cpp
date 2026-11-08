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

/*
 *  There is no MP3 encoder in GUCEF, so the streams of these tests are written here, frame by frame: MPEG 1 layer III
 *  frames at 44100 Hz and 64 kbit/s (208 bytes) whose spectrum is a few Huffman coded lines of table 1 with values
 *  that differ per frame. They are valid streams, a decoder that is right produces audio from them, but they are not
 *  a measure of how well a decoder reproduces music: that needs files from a real encoder, which come from
 *  GUCEF_TEST_AUDIO_ASSETS (see the last test).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>
#include <stdlib.h>

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
#include "TestAudioCodecMp3Support.h"
#include "TestAudioCodecMp3.h"

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
using namespace CodecTest::Mp3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* PluginGroup = "audiocodecpluginmp3";

/*-------------------------------------------------------------------------*/

// ---- ID3 tags ----

void
AppendSyncSafe( CORE::CDynamicBuffer& out, UInt32 value )
{
    const UInt8 bytes[ 4 ] = { (UInt8) ( ( value >> 21 ) & 0x7F ), (UInt8) ( ( value >> 14 ) & 0x7F ), (UInt8) ( ( value >> 7 ) & 0x7F ), (UInt8) ( value & 0x7F ) };
    out.Append( bytes, 4 );
}

/** A frame of ID3v2.3 or 2.4, the content is the bytes after the header */
void
AppendId3Frame( CORE::CDynamicBuffer& tag, UInt32 version, const char* id, const void* content, UInt32 size )
{
    if ( 2 == version )
    {
        tag.Append( id, 3 );
        const UInt8 sizeBytes[ 3 ] = { (UInt8) ( size >> 16 ), (UInt8) ( size >> 8 ), (UInt8) size };
        tag.Append( sizeBytes, 3 );
    }
    else
    {
        tag.Append( id, 4 );
        if ( 4 == version )
            AppendSyncSafe( tag, size );
        else
        {
            UInt8 sizeBytes[ 4 ];
            PutBE32( sizeBytes, size );
            tag.Append( sizeBytes, 4 );
        }
        const UInt8 flags[ 2 ] = { 0, 0 };
        tag.Append( flags, 2 );
    }
    tag.Append( content, size );
}

/*-------------------------------------------------------------------------*/

/** A text frame: the encoding byte and the bytes of the text */
void
AppendTextFrame( CORE::CDynamicBuffer& tag, UInt32 version, const char* id, UInt8 encoding, const void* text, UInt32 size )
{
    CORE::CDynamicBuffer content;
    content.Append( &encoding, 1 );
    content.Append( text, size );
    AppendId3Frame( tag, version, id, content.GetConstBufferPtr(), content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** The tag with its header around the frames; the size is that of the frames and 'padding' zero bytes */
void
WrapId3Tag( CORE::CDynamicBuffer& out, UInt32 version, UInt8 flags, const CORE::CDynamicBuffer& frames, UInt32 padding )
{
    const UInt8 header[ 6 ] = { 'I', 'D', '3', (UInt8) version, 0, flags };
    out.Append( header, 6 );
    AppendSyncSafe( out, frames.GetDataSize() + padding );
    out.Append( frames.GetConstBufferPtr(), frames.GetDataSize() );
    for ( UInt32 i=0; i<padding; ++i )
    {
        const UInt8 zero = 0;
        out.Append( &zero, 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
AppendBuffer( CORE::CDynamicBuffer& to, const CORE::CDynamicBuffer& from )
{
    to.Append( from.GetConstBufferPtr(), from.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioCodecMp3Tests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING audio MP3 codec plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.CodecMp3" );

    const bool loaded = LoadPlugin( "audiocodecpluginMP3", PluginGroup, "mp3" );

    GUCEF_TESTFW_TESTCASE( "CodecMp3: The plugin loads and registers the mp3 type, it decodes only and declares per source threading" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !loaded )
        GUCEF_TESTFW_SKIP( "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( loaded );
        TCodecPtr mp3 = FindCodec( "mp3" );
        ASSERT_FALSE( mp3.IsNULL() );
        ASSERT_TRUE( mp3->GetType() == "mp3" );
        ASSERT_FALSE( mp3->CanEncode() );
        TCodecPtr byName;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForFilename( "music/theme.MP3", byName ) );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == GetThreadingModel( "mp3" ) );

        CORE::CDynamicBuffer file;
        MakeStream( file, 1, 4, false );
        AUDIO::CAudioBuffer none;
        CORE::CDynamicBuffer output;
        CORE::CDynamicBufferAccess access( output );
        ASSERT_FALSE( mp3->Encode( AUDIO::CAudioBuffer( AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 100 ), access ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !loaded )
        return;

    TCodecPtr mp3 = FindCodec( "mp3" );

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Silent frames give silence, 1152 frames per MPEG frame, for mono and stereo" )
    try
    {
        for ( UInt32 channels=1; channels<=2; ++channels )
        {
            // frames without any spectrum: the header, the side information of zeros and no data
            CORE::CDynamicBuffer file;
            for ( UInt32 i=0; i<50; ++i )
            {
                UInt8 frame[ FrameBytes ];
                MakeInfoFrame( frame, channels, 0, 0, 0, false );
                memset( frame + 4, 0, FrameBytes - 4 );
                file.Append( frame, FrameBytes );
            }
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source ) );
            ASSERT_TRUE( source->GetFormat() == AUDIO::CAudioFormat( SampleRate, channels, AUDIO::SAMPLETYPE_FLOAT32 ) );
            ASSERT_TRUE( GUCEF_AUDIO_LENGTH_UNKNOWN == source->GetLengthInFrames() );
            AUDIO::CAudioBuffer audio;
            ASSERT_TRUE( ReadAll( *source, audio, 1000 ) );
            ASSERT_TRUE( 50 * FrameSamples == audio.GetFrameCount() );
            const Float32* samples = (const Float32*) audio.GetDataPtr();
            bool isSilent = true;
            for ( UInt32 i=0; i<audio.GetFrameCount() * channels; ++i )
                isSilent = isSilent && 0.0f == samples[ i ];
            ASSERT_TRUE( isSilent );
            ASSERT_TRUE( source->IsEndOfStream() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Frames with a spectrum decode to audio that is the same every time and moves with the content" )
    try
    {
        for ( UInt32 channels=1; channels<=2; ++channels )
        {
            CORE::CDynamicBuffer file;
            MakeStream( file, channels, 40, false );
            AUDIO::CAudioBuffer first, second;
            ASSERT_TRUE( DecodeFile( mp3, file, first ) );
            ASSERT_TRUE( DecodeFile( mp3, file, second ) );
            ASSERT_TRUE( 40 * FrameSamples == first.GetFrameCount() );
            ASSERT_TRUE( SameBytes( first, second ) );

            // not silent, nothing wild, and the channels are not the same
            Float32 peak = 0.0f;
            Float64 energy = 0.0;
            for ( UInt32 f=0; f<first.GetFrameCount(); ++f )
                for ( UInt32 c=0; c<channels; ++c )
                {
                    const Float32 v = first.GetSampleAsFloat( f, c );
                    peak = fabs( v ) > peak ? (Float32) fabs( v ) : peak;
                    energy += v * v;
                }
            ASSERT_TRUE( peak > 0.01f && peak < 1.0f && energy > 0.0 );
            if ( 2 == channels )
            {
                bool differ = false;
                for ( UInt32 f=0; f<first.GetFrameCount(); ++f )
                    differ = differ || first.GetSampleAsFloat( f, 0 ) != first.GetSampleAsFloat( f, 1 );
                ASSERT_TRUE( differ );
            }

            // another stream is another audio
            CORE::CDynamicBuffer other;
            MakeStream( other, channels, 40, false, 576, 1500, 2 );
            AUDIO::CAudioBuffer different;
            ASSERT_TRUE( DecodeFile( mp3, other, different ) );
            ASSERT_FALSE( SameBytes( first, different ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: The Info frame gives the length, and the delay and padding of the LAME tag are trimmed off the audio" )
    try
    {
        for ( UInt32 channels=1; channels<=2; ++channels )
        {
            const UInt32 delay = 576, padding = 1500, audioFrames = 40;
            CORE::CDynamicBuffer plain, withInfo;
            MakeStream( plain, channels, audioFrames, false );
            MakeStream( withInfo, channels, audioFrames, true, delay, padding );
            AUDIO::CAudioBuffer full, trimmed;
            ASSERT_TRUE( DecodeFile( mp3, plain, full ) );

            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, withInfo, source ) );
            const UInt64 expected = LengthWithInfo( audioFrames, delay, padding );
            ASSERT_TRUE( expected == source->GetLengthInFrames() );
            ASSERT_TRUE( ReadAll( *source, trimmed ) );
            ASSERT_TRUE( expected == trimmed.GetFrameCount() );

            // it is the same audio, from the end of the delay on: the Info frame is not audio
            ASSERT_TRUE( SameFrames( full, delay + 529, trimmed, 0, trimmed.GetFrameCount(), 0.0f ) );

            // the metadata of the codec says the same
            CORE::CValueList metaData;
            CORE::CDynamicBufferAccess input( withInfo );
            ASSERT_TRUE( mp3->DecodeMetaData( input, metaData ) );
            ASSERT_TRUE( metaData.GetValueAlways( "frames" ).AsString() == CORE::UInt64ToString( expected ) );

            // a Xing frame (variable bit rate) is read like an Info frame
            CORE::CDynamicBuffer xing;
            UInt8 xingFrame[ FrameBytes ];
            MakeInfoFrame( xingFrame, channels, audioFrames, delay, padding, true, true );
            xing.Append( xingFrame, FrameBytes );
            AppendFrames( xing, channels, 0, audioFrames, 1 );
            ASSERT_TRUE( OpenSource( mp3, xing, source ) );
            ASSERT_TRUE( expected == source->GetLengthInFrames() );

            // an Info frame without a LAME tag: whole frames
            CORE::CDynamicBuffer noLame;
            UInt8 infoFrame[ FrameBytes ];
            MakeInfoFrame( infoFrame, channels, audioFrames, 0, 0, false );
            noLame.Append( infoFrame, FrameBytes );
            AppendFrames( noLame, channels, 0, audioFrames, 1 );
            ASSERT_TRUE( OpenSource( mp3, noLame, source ) );
            ASSERT_TRUE( (UInt64) audioFrames * FrameSamples == source->GetLengthInFrames() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: A frame count in the Info frame that the file can not hold is not believed" )
    try
    {
        CORE::CDynamicBuffer file;
        UInt8 info[ FrameBytes ];
        MakeInfoFrame( info, 1, 0x7FFFFFFF, 0, 0, false );
        file.Append( info, FrameBytes );
        AppendFrames( file, 1, 0, 20, 1 );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( mp3, file, source ) );
        ASSERT_TRUE( GUCEF_AUDIO_LENGTH_UNKNOWN == source->GetLengthInFrames() );
        AUDIO::CAudioBuffer audio;
        ASSERT_TRUE( ReadAll( *source, audio ) );
        ASSERT_TRUE( 20 * FrameSamples == audio.GetFrameCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Seeking is exact, with and without the seek table made when the file is opened" )
    try
    {
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            // 0: no length, no table; 1: the length from an Info frame; 2: no Info frame, the file scanned when opened
            const UInt32 audioFrames = 120;
            CORE::CDynamicBuffer file;
            MakeStream( file, 2, audioFrames, 1 == variant );
            CORE::CValueList options;
            if ( 2 == variant )
                options.Set( "mp3.seekTable", "onOpen" );

            AUDIO::CAudioBuffer reference;
            {
                AUDIO::TAudioSourcePtr source;
                ASSERT_TRUE( OpenSource( mp3, file, source, options ) );
                ASSERT_TRUE( ReadAll( *source, reference ) );
            }
            const UInt32 length = reference.GetFrameCount();
            ASSERT_TRUE( length >= audioFrames * FrameSamples - 2200 );

            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source, options ) );
            if ( 0 == variant )
                ASSERT_TRUE( GUCEF_AUDIO_LENGTH_UNKNOWN == source->GetLengthInFrames() );
            else
                ASSERT_TRUE( length == source->GetLengthInFrames() );
            ASSERT_TRUE( source->IsSeekable() );

            CRandom random( 77 + variant );
            CORE::CDynamicBuffer chunk( 1024 * 8, true );
            AUDIO::CAudioBuffer reread;
            for ( UInt32 i=0; i<52; ++i )
            {
                UInt32 target = random.Below( length );
                if ( 0 == i ) target = 0;
                if ( 1 == i ) target = length - 1;
                if ( 2 == i ) target = FrameSamples;
                if ( 3 == i ) target = FrameSamples - 1;
                if ( 4 == i ) target = 30 * FrameSamples + 5;
                ASSERT_TRUE( source->SeekToFrame( target ) );
                const UInt32 expected = length - target < 1024 ? length - target : 1024;
                ASSERT_TRUE( expected == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
                const AUDIO::CAudioBuffer got( reference.GetFormat(), chunk.GetConstBufferPtr(), expected );
                // bit exact is what is expected; a decoder that warms up its filters after a seek would be a few bits off
                ASSERT_TRUE( SameFrames( reference, target, got, 0, expected, 1.0e-6f ) );
            }

            // the end
            ASSERT_TRUE( source->SeekToFrame( length ) );
            ASSERT_TRUE( 0 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
            ASSERT_TRUE( source->IsEndOfStream() );
            if ( 0 != variant )
                ASSERT_FALSE( source->SeekToFrame( length + 1 ) );
            else
                source->SeekToFrame( length + 1 );
            ASSERT_TRUE( source->Rewind() );
            ASSERT_TRUE( 1024 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: A stream that loops three times has no gap at the seams when the file has a LAME tag" )
    try
    {
        CORE::CDynamicBuffer file;
        MakeStream( file, 2, 30, true );
        AUDIO::CAudioBuffer reference;
        ASSERT_TRUE( DecodeFile( mp3, file, reference ) );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( mp3, file, source ) );
        AUDIO::CAudioBuffer played;
        UInt64 underruns = 1;
        ASSERT_TRUE( PlayLooped( source, 3, played, underruns ) );
        const UInt32 length = reference.GetFrameCount();
        ASSERT_TRUE( 3 * length == played.GetFrameCount() );
        ASSERT_TRUE( 0 == underruns );
        for ( UInt32 loop=0; loop<3; ++loop )
            ASSERT_TRUE( SameFrames( reference, 0, played, loop * length, length, 0.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Tags of ID3v2.2, 2.3 and 2.4 in all their encodings, and of ID3v1, reach the stream and the metadata" )
    try
    {
        CORE::CDynamicBuffer audioFrames;
        MakeStream( audioFrames, 1, 6, true );

        // 2.3: Latin-1, UTF-16 with a byte order mark, a user text frame, a frame that is not text, unknown text frame
        {
            CORE::CDynamicBuffer frames, tag, file;
            AppendTextFrame( frames, 3, "TIT2", 0, "T\xEB" "st \xFF", 6 );
            const UInt8 artist[] = { 0xFF, 0xFE, 'A', 0, 0xE9, 0x00, 0x3D, 0xD8, 0x00, 0xDE };        // "A", e acute, and U+1F600 as a surrogate pair
            AppendTextFrame( frames, 3, "TPE1", 1, artist, sizeof( artist ) );
            AppendTextFrame( frames, 3, "TALB", 0, "Album", 5 );
            AppendTextFrame( frames, 3, "TRCK", 0, "3/12", 4 );
            AppendTextFrame( frames, 3, "TYER", 0, "1999", 4 );
            AppendTextFrame( frames, 3, "TCON", 0, "Rock", 4 );
            AppendTextFrame( frames, 3, "TPOS", 0, "1/2", 3 );
            const UInt8 loop[] = { 0, 'L', 'O', 'O', 'P', 'S', 'T', 'A', 'R', 'T', 0, '4', '4', '1', '0', '0' };
            AppendId3Frame( frames, 3, "TXXX", loop, sizeof( loop ) );
            AppendId3Frame( frames, 3, "APIC", "\0image/png\0\3\0not really a picture", 31 );
            AppendTextFrame( frames, 3, "TSSE", 0, "unknown", 7 );
            WrapId3Tag( tag, 3, 0, frames, 30 );
            AppendBuffer( file, tag );
            AppendBuffer( file, audioFrames );

            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "T\xC3\xAB" "st \xC3\xBF" );
            ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "A\xC3\xA9\xF0\x9F\x98\x80" );
            ASSERT_TRUE( tags.GetValueAlways( "album" ).AsString() == "Album" );
            ASSERT_TRUE( tags.GetValueAlways( "track" ).AsString() == "3/12" );
            ASSERT_TRUE( tags.GetValueAlways( "date" ).AsString() == "1999" );
            ASSERT_TRUE( tags.GetValueAlways( "genre" ).AsString() == "Rock" );
            ASSERT_TRUE( tags.GetValueAlways( "disc" ).AsString() == "1/2" );
            ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "44100" );
            ASSERT_TRUE( 8 == tags.GetKeyCount() );

            // the tag does not disturb the audio, nor the positions
            CORE::CDynamicBuffer plain;
            AppendBuffer( plain, audioFrames );
            AUDIO::CAudioBuffer withTag, withoutTag;
            ASSERT_TRUE( DecodeFile( mp3, file, withTag ) );
            ASSERT_TRUE( DecodeFile( mp3, plain, withoutTag ) );
            ASSERT_TRUE( SameBytes( withTag, withoutTag ) );
            ASSERT_TRUE( source->SeekToFrame( 2000 ) );
            Float32 frames2[ 100 ];
            ASSERT_TRUE( 100 == source->ReadFrames( frames2, 100 ) );
            ASSERT_TRUE( 0 == memcmp( frames2, withoutTag.GetDataPtr( 2000 ), 400 ) );

            CORE::CValueList metaData;
            CORE::CDynamicBufferAccess input( file );
            ASSERT_TRUE( mp3->DecodeMetaData( input, metaData ) );
            ASSERT_TRUE( metaData.GetValueAlways( "title" ).AsString() == "T\xC3\xAB" "st \xC3\xBF" );
            ASSERT_TRUE( metaData.GetValueAlways( "sampleRate" ).AsString() == "44100" );
            ASSERT_TRUE( metaData.GetValueAlways( "channels" ).AsString() == "1" );
        }

        // 2.4: sync safe sizes, UTF-8, UTF-16 big endian, a user text frame and the value after the first of several
        {
            CORE::CDynamicBuffer frames, tag, file;
            AppendTextFrame( frames, 4, "TIT2", 3, "Caf\xC3\xA9", 5 );
            const UInt8 artist[] = { 0x00, 'B', 0x00, 0xE9 };
            AppendTextFrame( frames, 4, "TPE1", 2, artist, sizeof( artist ) );
            AppendTextFrame( frames, 4, "TDRC", 3, "2024-05-06", 10 );
            AppendTextFrame( frames, 4, "TALB", 3, "First\0Second", 12 );                // two values: the first is used
            const UInt8 loop[] = { 3, 'l', 'o', 'o', 'p', 'l', 'e', 'n', 'g', 't', 'h', 0, '9', '9' };
            AppendId3Frame( frames, 4, "TXXX", loop, sizeof( loop ) );
            WrapId3Tag( tag, 4, 0, frames, 0 );
            AppendBuffer( file, tag );
            AppendBuffer( file, audioFrames );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Caf\xC3\xA9" );
            ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "B\xC3\xA9" );
            ASSERT_TRUE( tags.GetValueAlways( "date" ).AsString() == "2024-05-06" );
            ASSERT_TRUE( tags.GetValueAlways( "album" ).AsString() == "First" );
            ASSERT_TRUE( tags.GetValueAlways( "looplength" ).AsString() == "99" );
        }

        // 2.2: three letter names
        {
            CORE::CDynamicBuffer frames, tag, file;
            AppendTextFrame( frames, 2, "TT2", 0, "Old", 3 );
            AppendTextFrame( frames, 2, "TP1", 0, "Timer", 5 );
            AppendTextFrame( frames, 2, "TRK", 0, "7", 1 );
            WrapId3Tag( tag, 2, 0, frames, 4 );
            AppendBuffer( file, tag );
            AppendBuffer( file, audioFrames );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Old" );
            ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "Timer" );
            ASSERT_TRUE( tags.GetValueAlways( "track" ).AsString() == "7" );
        }

        // unsynchronisation of the whole tag: after every 0xFF a zero is put, which the reader takes out
        {
            CORE::CDynamicBuffer frames, unsynced, tag, file;
            AppendTextFrame( frames, 3, "TIT2", 0, "A\xFF" "B", 3 );
            const UInt8* bytes = (const UInt8*) frames.GetConstBufferPtr();
            for ( UInt32 i=0; i<frames.GetDataSize(); ++i )
            {
                unsynced.Append( bytes + i, 1 );
                if ( 0xFF == bytes[ i ] )
                {
                    const UInt8 zero = 0;
                    unsynced.Append( &zero, 1 );
                }
            }
            WrapId3Tag( tag, 3, 0x80, unsynced, 0 );
            AppendBuffer( file, tag );
            AppendBuffer( file, audioFrames );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "A\xC3\xBF" "B" );
        }

        // ID3v1 at the end, and an ID3v2 title that wins over it
        {
            CORE::CDynamicBuffer v1, file, frames, tag;
            UInt8 id3v1[ 128 ];
            memset( id3v1, 0, sizeof( id3v1 ) );
            memcpy( id3v1, "TAG", 3 );
            memcpy( id3v1 + 3, "Version One", 11 );
            memcpy( id3v1 + 33, "Some Artist   ", 14 );
            memcpy( id3v1 + 63, "An Album", 8 );
            memcpy( id3v1 + 93, "1987", 4 );
            memcpy( id3v1 + 97, "short comment", 13 );
            id3v1[ 126 ] = 9;                       // track number of version 1.1
            id3v1[ 127 ] = 17;
            AppendBuffer( file, audioFrames );
            file.Append( id3v1, 128 );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, file, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Version One" );
            ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "Some Artist" );
            ASSERT_TRUE( tags.GetValueAlways( "album" ).AsString() == "An Album" );
            ASSERT_TRUE( tags.GetValueAlways( "date" ).AsString() == "1987" );
            ASSERT_TRUE( tags.GetValueAlways( "track" ).AsString() == "9" );
            ASSERT_TRUE( tags.GetValueAlways( "comment" ).AsString() == "short comment" );

            // the audio stops where the tag starts
            AUDIO::CAudioBuffer audio;
            ASSERT_TRUE( ReadAll( *source, audio ) );
            ASSERT_TRUE( LengthWithInfo( 6, 576, 1500 ) == audio.GetFrameCount() );

            AppendTextFrame( frames, 3, "TIT2", 0, "Version Two", 11 );
            WrapId3Tag( tag, 3, 0, frames, 0 );
            CORE::CDynamicBuffer both;
            AppendBuffer( both, tag );
            AppendBuffer( both, file );
            ASSERT_TRUE( OpenSource( mp3, both, source ) );
            tags.Clear();
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Version Two" );
            ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "Some Artist" );
        }

        // no tag, no tags
        {
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( mp3, audioFrames, source ) );
            CORE::CValueList tags;
            ASSERT_FALSE( source->GetMetaData( tags ) );
            ASSERT_TRUE( 0 == tags.GetKeyCount() );
        }

        // a tag that claims to be enormous is skipped, the audio is still found
        {
            CORE::CDynamicBuffer file;
            const UInt8 header[ 10 ] = { 'I', 'D', '3', 3, 0, 0, 0x7F, 0x7F, 0x7F, 0x7F };       // 256 MB
            file.Append( header, 10 );
            AppendBuffer( file, audioFrames );
            AUDIO::TAudioSourcePtr source;
            ASSERT_FALSE( OpenSource( mp3, file, source ) );          // there is no more file, so nothing to decode, and no huge allocation
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: The probe knows MPEG audio frames, also behind a tag, and nothing else" )
    try
    {
        CORE::CDynamicBuffer file;
        MakeStream( file, 2, 10, true );
        CORE::CDynamicBufferAccess input( file );
        UInt32 confidence = 0;
        ASSERT_TRUE( mp3->Probe( input, confidence ) );
        ASSERT_TRUE( 95 == confidence );
        ASSERT_TRUE( 0 == input.Tell() );

        TCodecPtr found;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( input, found, &confidence ) );
        ASSERT_TRUE( found->GetType() == "mp3" && 95 == confidence );

        // behind a tag
        CORE::CDynamicBuffer frames, tag, tagged;
        AppendTextFrame( frames, 3, "TIT2", 0, "x", 1 );
        WrapId3Tag( tag, 3, 0, frames, 10 );
        AppendBuffer( tagged, tag );
        AppendBuffer( tagged, file );
        CORE::CDynamicBufferAccess taggedInput( tagged );
        ASSERT_TRUE( mp3->Probe( taggedInput, confidence ) );
        ASSERT_TRUE( 95 == confidence );

        // a tag and no audio after it, a single frame, a few bytes of junk in front
        CORE::CDynamicBuffer onlyTag;
        AppendBuffer( onlyTag, tag );
        CORE::CDynamicBufferAccess onlyTagInput( onlyTag );
        ASSERT_TRUE( mp3->Probe( onlyTagInput, confidence ) );
        ASSERT_TRUE( 20 == confidence );
        CORE::CDynamicBuffer single;
        UInt8 frame[ FrameBytes ];
        MakeFrame( frame, 1, 0, 1 );
        single.Append( frame, FrameBytes );
        CORE::CDynamicBufferAccess singleInput( single );
        ASSERT_TRUE( mp3->Probe( singleInput, confidence ) );
        ASSERT_TRUE( 70 == confidence );
        CORE::CDynamicBuffer junked;
        junked.Append( "JUNKJUNK", 8 );
        AppendBuffer( junked, file );
        CORE::CDynamicBufferAccess junkedInput( junked );
        ASSERT_TRUE( mp3->Probe( junkedInput, confidence ) );
        ASSERT_TRUE( 85 == confidence );

        // not MP3: noise, text, a WAVE header, a layer II frame
        CRandom random( 4 );
        CORE::CDynamicBuffer noise( 5000, true );
        for ( UInt32 i=0; i<5000; ++i )
            ( (UInt8*) noise.GetBufferPtr() )[ i ] = (UInt8) ( random.Next() & 0x7F );     // no byte of 0xFF at all
        CORE::CDynamicBufferAccess noiseInput( noise );
        ASSERT_TRUE( mp3->Probe( noiseInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );
        ASSERT_FALSE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( noiseInput, found ) );
        CORE::CDynamicBuffer layer2;
        UInt8 layer2Frame[ FrameBytes ];
        memcpy( layer2Frame, frame, FrameBytes );
        layer2Frame[ 1 ] = 0xFD;        // layer II
        layer2.Append( layer2Frame, FrameBytes );
        layer2.Append( layer2Frame, FrameBytes );
        CORE::CDynamicBufferAccess layer2Input( layer2 );
        ASSERT_TRUE( mp3->Probe( layer2Input, confidence ) );
        ASSERT_TRUE( 0 == confidence );
        CORE::CDynamicBuffer tiny( "\xFF", 1, true );
        CORE::CDynamicBufferAccess tinyInput( tiny );
        ASSERT_TRUE( mp3->Probe( tinyInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Truncated and corrupted streams never crash or hang the decoder" )
    try
    {
        CORE::CDynamicBuffer frames, tag, file, stream;
        MakeStream( stream, 2, 10, true );
        AppendTextFrame( frames, 3, "TIT2", 0, "Sweep", 5 );
        const UInt8 loop[] = { 0, 'L', 'O', 'O', 'P', 0, '1', '2' };
        AppendId3Frame( frames, 3, "TXXX", loop, sizeof( loop ) );
        WrapId3Tag( tag, 3, 0, frames, 0 );
        AppendBuffer( file, tag );
        AppendBuffer( file, stream );
        UInt8 id3v1[ 128 ];
        memset( id3v1, 0, sizeof( id3v1 ) );
        memcpy( id3v1, "TAG", 3 );
        file.Append( id3v1, 128 );
        ASSERT_TRUE( SweepDamage( mp3, file, 1, 1000, 100000000, 31 ) == 0 );

        // and the same with the seek table made when the file is opened
        CORE::CValueList options;
        options.Set( "mp3.seekTable", "onOpen" );
        for ( UInt32 length=0; length<=file.GetDataSize(); length += 13 )
        {
            CORE::CDynamicBuffer cut( (const char*) file.GetConstBufferPtr(), length, true );
            AUDIO::TAudioSourcePtr source;
            if ( OpenSource( mp3, cut, source, options ) )
            {
                AUDIO::CAudioBuffer audio;
                ASSERT_TRUE( ReadAll( *source, audio ) );
                source->SeekToFrame( audio.GetFrameCount() / 2 );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Two streams of the plugin decode at the same time, per source" )
    try
    {
        CORE::CDynamicBuffer file;
        MakeStream( file, 2, 400, true );
        AUDIO::CAudioBuffer reference;
        ASSERT_TRUE( DecodeFile( mp3, file, reference ) );
        Int32 maxOverlap = 0;
        ASSERT_TRUE( DecodeTwoAtOnce( mp3, file, reference, maxOverlap ) );
        ASSERT_TRUE( 2 == maxOverlap );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Writes WAVE files of generated signals and the hand made MP3 files into GUCEF_TEST_AUDIO_WRITE_SIGNALS (opt in)" )
    {
        const CORE::CString directory( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_WRITE_SIGNALS" ).c_str() );
        GUCEF_TESTFW_SKIP_IF( directory.IsNULLOrEmpty(), "GUCEF_TEST_AUDIO_WRITE_SIGNALS is not set" );
        try
        {
            // The WAVE files are what the owner encodes with the reference encoders (LAME with its tag, oggenc, opusenc)
            // and puts back in GUCEF_TEST_AUDIO_ASSETS: mp3/<name>.mp3 with a line in mp3/manifest.txt
            const CORE::CString signals = CORE::CombinePath( directory, "signals" );
            ASSERT_TRUE( CORE::CreatePathDirectories( CORE::CombinePath( signals, "file" ) ) );
            TCodecPtr wav = FindCodec( "wav" );
            ASSERT_FALSE( wav.IsNULL() );
            const AUDIO::CAudioFormat formats[] = { AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_INT16 ), AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_INT16 ) };
            const char* names[] = { "signal-mono-44100.wav", "signal-stereo-48000.wav", "signal-six-channel-48000.wav" };
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CDynamicBuffer file;
                ASSERT_TRUE( Encode( wav, MakeTones( formats[ i ], formats[ i ].GetSampleRate() * 3, 500.0, 0.5 ), file ) );
                ASSERT_TRUE( file.WriteContentToFile( CORE::CombinePath( signals, names[ i ] ), 0, true ) );
            }

            // MP3 files that this test made, with the manifest lines of what they have to give
            const CORE::CString mp3Directory = CORE::CombinePath( directory, "mp3" );
            ASSERT_TRUE( CORE::CreatePathDirectories( CORE::CombinePath( mp3Directory, "file" ) ) );
            CORE::CDynamicBuffer manifest;
            const char* header = "# file  length in frames ( - when the file does not tell )  sample rate  channels\n";
            manifest.Append( header, (UInt32) strlen( header ) );
            for ( UInt32 variant=0; variant<4; ++variant )
            {
                CORE::CDynamicBuffer file;
                MakeStream( file, 0 == variant % 2 ? 1 : 2, 80, variant >= 2 );
                const CORE::CString name = CORE::CString( "handmade-" ) + CORE::UInt32ToString( variant ) + ".mp3";
                ASSERT_TRUE( file.WriteContentToFile( CORE::CombinePath( mp3Directory, name ), 0, true ) );

                // and what the plugin makes of it, 32 bit float samples as they come, to compare with other decoders
                AUDIO::CAudioBuffer decoded;
                ASSERT_TRUE( DecodeFile( mp3, file, decoded ) );
                CORE::CDynamicBuffer raw( (const char*) decoded.GetDataPtr(), decoded.GetSizeInBytes(), true );
                ASSERT_TRUE( raw.WriteContentToFile( CORE::CombinePath( mp3Directory, name + ".decoded.f32" ), 0, true ) );
                const CORE::CString line = name + " " + ( variant >= 2 ? CORE::UInt64ToString( LengthWithInfo( 80, 576, 1500 ) ) : CORE::CString( "-" ) ) +
                                           " 44100 " + CORE::UInt32ToString( 0 == variant % 2 ? 1 : 2 ) + "\n";
                manifest.Append( line.C_String(), line.Length() );
            }
            ASSERT_TRUE( manifest.WriteContentToFile( CORE::CombinePath( mp3Directory, "manifest.txt" ), 0, true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: The MP3 files in GUCEF_TEST_AUDIO_ASSETS/mp3 decode as the manifest says (opt in, needs files made by a real encoder)" )
    {
        const CORE::CString assets( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_ASSETS" ).c_str() );
        GUCEF_TESTFW_SKIP_IF( assets.IsNULLOrEmpty(), "GUCEF_TEST_AUDIO_ASSETS is not set" );
        const CORE::CString directory = CORE::CombinePath( assets, "mp3" );
        const CORE::CString manifestPath = CORE::CombinePath( directory, "manifest.txt" );
        GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( manifestPath ), "there is no mp3/manifest.txt in GUCEF_TEST_AUDIO_ASSETS" );
        try
        {
            CORE::CDynamicBuffer manifest;
            ASSERT_TRUE( manifest.LoadContentFromFile( manifestPath ) );
            const CORE::CString text( (const char*) manifest.GetConstBufferPtr(), manifest.GetDataSize() );
            const CORE::CString::StringVector lines = text.ParseElements( '\n', false );
            UInt32 checked = 0;
            for ( size_t i=0; i<lines.size(); ++i )
            {
                CORE::CString line = lines[ i ].Trim( true ).Trim( false );
                if ( line.IsNULLOrEmpty() || '#' == line[ 0 ] )
                    continue;
                const CORE::CString::StringVector fields = line.ParseElements( ' ', false );
                ASSERT_TRUE( 4 == fields.size() );
                if ( 4 != fields.size() )
                    continue;

                CORE::CDynamicBuffer file;
                ASSERT_TRUE( file.LoadContentFromFile( CORE::CombinePath( directory, fields[ 0 ] ) ) );
                AUDIO::TAudioSourcePtr source;
                ASSERT_TRUE( OpenSource( mp3, file, source ) );
                ASSERT_TRUE( CORE::StringToUInt32( fields[ 2 ] ) == source->GetFormat().GetSampleRate() );
                ASSERT_TRUE( CORE::StringToUInt32( fields[ 3 ] ) == source->GetFormat().GetChannelCount() );

                AUDIO::CAudioBuffer audio;
                ASSERT_TRUE( ReadAll( *source, audio ) );
                if ( fields[ 1 ] != "-" )
                {
                    ASSERT_TRUE( CORE::StringToUInt64( fields[ 1 ] ) == audio.GetFrameCount() );
                    ASSERT_TRUE( CORE::StringToUInt64( fields[ 1 ] ) == source->GetLengthInFrames() );
                }
                ASSERT_TRUE( audio.GetFrameCount() > 0 );

                // not silence and not noise from a broken decode: the peak is a sensible one
                Float32 peak = 0.0f;
                for ( UInt32 f=0; f<audio.GetFrameCount(); ++f )
                    for ( UInt32 c=0; c<audio.GetFormat().GetChannelCount(); ++c )
                        peak = (Float32) fabs( audio.GetSampleAsFloat( f, c ) ) > peak ? (Float32) fabs( audio.GetSampleAsFloat( f, c ) ) : peak;
                ASSERT_TRUE( peak > 0.001f && peak < 1.5f );

                // seeks give what reading on gives
                CRandom random( 5 + i );
                CORE::CDynamicBuffer chunk( 1024 * audio.GetFormat().GetBytesPerFrame(), true );
                for ( UInt32 n=0; n<12; ++n )
                {
                    const UInt32 target = random.Below( audio.GetFrameCount() );
                    ASSERT_TRUE( source->SeekToFrame( target ) );
                    const UInt32 expected = audio.GetFrameCount() - target < 1024 ? audio.GetFrameCount() - target : 1024;
                    ASSERT_TRUE( expected == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
                    const AUDIO::CAudioBuffer got( audio.GetFormat(), chunk.GetConstBufferPtr(), expected );
                    ASSERT_TRUE( SameFrames( audio, target, got, 0, expected, 1.0e-4f ) );
                }
                ++checked;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MP3 asset files checked: " << checked );
            ASSERT_TRUE( checked > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecMp3: Unloading the plugin takes the type away" )
    try
    {
        mp3 = TCodecPtr();
        ASSERT_TRUE( UnloadPlugin( PluginGroup, "mp3" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
