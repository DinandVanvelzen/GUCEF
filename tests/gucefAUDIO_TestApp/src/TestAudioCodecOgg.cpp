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
#include "TestAudioCodecOgg.h"

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

const char* PluginGroup = "audiocodecpluginogg";

/*-------------------------------------------------------------------------*/

bool
EncodeVorbis( TCodecPtr ogg, const AUDIO::CAudioBuffer& audio, CORE::CDynamicBuffer& file, const char* quality = "1.0", const char* serial = GUCEF_NULL, const char* tags = GUCEF_NULL )
{
    CORE::CValueList options;
    options.Set( "quality", quality );
    if ( GUCEF_NULL != serial )
        options.Set( "serial", serial );
    if ( GUCEF_NULL != tags )
        options.Set( "tags", tags );
    return Encode( ogg, audio, file, options );
}

/*-------------------------------------------------------------------------*/

bool
EncodeOpus( TCodecPtr opus, const AUDIO::CAudioBuffer& audio, CORE::CDynamicBuffer& file, const char* bitrate = "256000", const char* serial = GUCEF_NULL, const char* tags = GUCEF_NULL )
{
    CORE::CValueList options;
    options.Set( "bitrate", bitrate );
    if ( GUCEF_NULL != serial )
        options.Set( "serial", serial );
    if ( GUCEF_NULL != tags )
        options.Set( "tags", tags );
    return Encode( opus, audio, file, options );
}

/*-------------------------------------------------------------------------*/

void
AppendFile( CORE::CDynamicBuffer& to, const CORE::CDynamicBuffer& from )
{
    to.Append( from.GetConstBufferPtr(), from.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/**
 *  A lossy codec returned what went in when: the length is exact, a tone has the level it had to within half a
 *  decibel and the signal to noise ratio is at least 'minSnrInDb'.
 *  'snr' gets the lowest SNR of the channels.
 */
bool
LossyMatches( const AUDIO::CAudioBuffer& reference, const AUDIO::CAudioBuffer& got, Float64 baseFrequency, Float64 minSnrInDb, Float64& snr )
{
    snr = 1000.0;
    if ( reference.GetFrameCount() != got.GetFrameCount() || reference.GetFormat().GetChannelCount() != got.GetFormat().GetChannelCount() ||
         reference.GetFormat().GetSampleRate() != got.GetFormat().GetSampleRate() )
        return false;

    const UInt32 length = reference.GetFrameCount();
    bool isOk = length > 12000;
    for ( UInt32 c=0; c<reference.GetFormat().GetChannelCount() && isOk; ++c )
    {
        const Float64 frequency = ToneOfChannel( c, baseFrequency );
        const Float64 levelReference = ToneAmplitude( reference, c, frequency, 4000, 6000 );
        const Float64 levelGot = ToneAmplitude( got, c, frequency, 4000, 6000 );
        isOk = fabs( ToDecibel( levelGot / levelReference ) ) < 0.5 && isOk;

        const Float64 channelSnr = SnrInDb( reference, got, c, 0, 2000, length - 4000 );
        if ( channelSnr < snr )
            snr = channelSnr;
    }
    return isOk && snr >= minSnrInDb;
}

/*-------------------------------------------------------------------------*/

/**
 *  The audio is not shifted: the best match of the decoded noise with the noise that went in is at lag 0. Noise,
 *  as a tone matches itself at every period
 */
bool
IsNotShifted( const AUDIO::CAudioBuffer& noise, const AUDIO::CAudioBuffer& got )
{
    for ( UInt32 c=0; c<noise.GetFormat().GetChannelCount(); ++c )
        if ( 0 != BestLag( noise, got, c, 800, 4000, 6000 ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Tones for the channels: a different one per channel, and a low one for the low frequency channel of a surround layout, as codecs may filter that one */
Float64
ChannelTone( UInt32 channel, UInt32 channels )
{
    return ( channel == 3 && channels >= 6 ) ? 50.0 : 100.0 * (Float64) ( channel + 1 );
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioBuffer
MakeChannelTones( const AUDIO::CAudioFormat& format, UInt32 frames, Float64 amplitude )
{
    AUDIO::CAudioBuffer audio( format, frames );
    for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
    {
        const Float64 step = 6.283185307179586476925286766559 * ChannelTone( c, format.GetChannelCount() ) / (Float64) format.GetSampleRate();
        for ( UInt32 f=0; f<frames; ++f )
            audio.SetSampleFromFloat( f, c, (Float32) ( amplitude * sin( step * (Float64) f ) ) );
    }
    return audio;
}

/*-------------------------------------------------------------------------*/

/** Which channel has the tone of channel 'c' of a layout of 'channels' channels the most */
UInt32
WhereIsTone( const AUDIO::CAudioBuffer& audio, UInt32 c, UInt32 channels )
{
    return ChannelWithTone( audio, ChannelTone( c, channels ), 5000, 16000 );
}

/*-------------------------------------------------------------------------*/

/** The first page of an Ogg stream with the given first packet, as far as the plugin looks at it (no checksum needed) */
void
MakeOggStart( CORE::CDynamicBuffer& file, const void* packet, UInt32 packetSize )
{
    file.Clear();
    UInt8 header[ 27 ];
    memset( header, 0, sizeof( header ) );
    memcpy( header, "OggS", 4 );
    header[ 5 ] = 0x02;             // first page of a stream
    header[ 26 ] = 1;               // one segment
    file.Append( header, 27 );
    const UInt8 segment = (UInt8) packetSize;
    file.Append( &segment, 1 );
    file.Append( packet, packetSize );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioCodecOggTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING audio Ogg codec plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.CodecOgg" );

    const bool loaded = LoadPlugin( "audiocodecpluginOGG", PluginGroup, "ogg" );

    GUCEF_TESTFW_TESTCASE( "CodecOgg: The plugin loads and registers the ogg, oga and opus types and declares per source threading" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !loaded )
        GUCEF_TESTFW_SKIP( "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( loaded );
        ASSERT_FALSE( FindCodec( "ogg" ).IsNULL() );
        ASSERT_FALSE( FindCodec( "oga" ).IsNULL() );
        ASSERT_FALSE( FindCodec( "opus" ).IsNULL() );
        ASSERT_TRUE( FindCodec( "OGG" )->CanEncode() );
        TCodecPtr byName;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForFilename( "music/theme.Ogg", byName ) );
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForFilename( "voice/line.opus", byName ) );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == GetThreadingModel( "ogg" ) );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == GetThreadingModel( "opus" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !loaded )
        return;

    TCodecPtr ogg = FindCodec( "ogg" );
    TCodecPtr oga = FindCodec( "oga" );
    TCodecPtr opus = FindCodec( "opus" );

    // The Opus half is only there where the opus libraries are
    bool hasOpus = false;
    {
        CORE::CDynamicBuffer probe;
        hasOpus = EncodeOpus( opus, MakeTones( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_INT16 ), 4800, 500.0, 0.5 ), probe );
    }

    // ---- Vorbis ----

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Vorbis round trips: exact length, no shift, level within half a decibel, SNR of at least 30 dB at high quality" )
    try
    {
        const UInt32 rates[] = { 8000, 22050, 44100, 48000 };
        const UInt32 channelCounts[] = { 1, 2, 1, 2 };
        for ( UInt32 i=0; i<4; ++i )
        {
            const AUDIO::CAudioFormat format( rates[ i ], channelCounts[ i ], AUDIO::SAMPLETYPE_INT16 );
            const AUDIO::CAudioBuffer audio = MakeTones( format, rates[ i ] * 2 + 123, 1000.0 * rates[ i ] / 44100.0, 0.5 );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeVorbis( ogg, audio, file ) );

            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( ogg, file, source ) );
            ASSERT_TRUE( source->GetFormat() == AUDIO::CAudioFormat( rates[ i ], channelCounts[ i ], AUDIO::SAMPLETYPE_FLOAT32 ) );
            ASSERT_TRUE( audio.GetFrameCount() == source->GetLengthInFrames() );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( ReadAll( *source, decoded ) );
            Float64 snr = 0.0;
            ASSERT_TRUE( LossyMatches( audio, decoded, 1000.0 * rates[ i ] / 44100.0, 30.0, snr ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Vorbis " << rates[ i ] << " Hz " << channelCounts[ i ] << " ch: SNR " << snr << " dB, " << file.GetDataSize() << " bytes" );

            // and the audio is where it was: noise, which matches itself at one place only
            const AUDIO::CAudioBuffer noise = MakeNoise( format, 20000, 0.3f, 9 );
            CORE::CDynamicBuffer noiseFile;
            AUDIO::CAudioBuffer noiseDecoded;
            ASSERT_TRUE( EncodeVorbis( ogg, noise, noiseFile ) );
            ASSERT_TRUE( DecodeFile( ogg, noiseFile, noiseDecoded ) );
            ASSERT_TRUE( 20000 == noiseDecoded.GetFrameCount() );
            ASSERT_TRUE( IsNotShifted( noise, noiseDecoded ) );
        }

        // the names of the family are one codec, and a lower quality makes a smaller file
        const AUDIO::CAudioBuffer music = MakeNoise( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), 44100, 0.2f, 5 );
        CORE::CDynamicBuffer low, high, viaOga;
        ASSERT_TRUE( EncodeVorbis( ogg, music, low, "0.1" ) );
        ASSERT_TRUE( EncodeVorbis( ogg, music, high, "0.9" ) );
        ASSERT_TRUE( EncodeVorbis( oga, music, viaOga, "0.9" ) );
        ASSERT_TRUE( low.GetDataSize() < high.GetDataSize() );
        ASSERT_TRUE( viaOga.GetDataSize() == high.GetDataSize() );
        CORE::CValueList bitrate;
        bitrate.Set( "bitrate", "64000" );
        CORE::CDynamicBuffer abr;
        ASSERT_TRUE( Encode( ogg, music, abr, bitrate ) );
        ASSERT_TRUE( abr.GetDataSize() > 1000 );

        // what the format can not do is turned down
        CORE::CDynamicBuffer rejected;
        ASSERT_FALSE( Encode( ogg, MakeTones( AUDIO::CAudioFormat( 44100, 9, AUDIO::SAMPLETYPE_INT16 ), 1000, 500.0, 0.5 ), rejected ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Vorbis seeking is exact: 50 random places, the start, the last frame and the end" )
    try
    {
        AUDIO::CAudioBuffer audio = MakeNoise( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), 3 * 44100 + 777, 0.15f, 3 );
        const AUDIO::CAudioBuffer tone = MakeTones( audio.GetFormat(), audio.GetFrameCount(), 330.0, 0.4 );
        for ( UInt32 i=0; i<audio.GetFrameCount() * 2; ++i )
            ( (Int16*) audio.GetDataPtr() )[ i ] = (Int16) ( ( (Int16*) audio.GetDataPtr() )[ i ] + ( (const Int16*) tone.GetDataPtr() )[ i ] );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( EncodeVorbis( ogg, audio, file, "0.5" ) );

        AUDIO::CAudioBuffer reference;
        ASSERT_TRUE( DecodeFile( ogg, file, reference ) );
        const UInt32 length = reference.GetFrameCount();
        ASSERT_TRUE( audio.GetFrameCount() == length );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( ogg, file, source ) );
        ASSERT_TRUE( source->IsSeekable() );
        CRandom random( 4242 );
        CORE::CDynamicBuffer chunk( 1024 * 8, true );
        for ( UInt32 i=0; i<54; ++i )
        {
            UInt32 target = random.Below( length );
            if ( 0 == i ) target = 0;
            if ( 1 == i ) target = length - 1;
            if ( 2 == i ) target = 2048;
            if ( 3 == i ) target = 2047;
            ASSERT_TRUE( source->SeekToFrame( target ) );
            const UInt32 expected = length - target < 1024 ? length - target : 1024;
            ASSERT_TRUE( expected == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
            ASSERT_TRUE( 0 == memcmp( chunk.GetConstBufferPtr(), reference.GetDataPtr( target ), (size_t) expected * 8 ) );
        }
        ASSERT_TRUE( source->SeekToFrame( length ) );
        ASSERT_TRUE( 0 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
        ASSERT_TRUE( source->IsEndOfStream() );
        ASSERT_FALSE( source->SeekToFrame( length + 1 ) );
        ASSERT_TRUE( source->Rewind() );
        ASSERT_TRUE( 1024 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
        ASSERT_TRUE( 0 == memcmp( chunk.GetConstBufferPtr(), reference.GetDataPtr(), 8192 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: A Vorbis stream that loops three times has no gap at the seams" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeTones( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), 30001, 600.0, 0.4 );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( EncodeVorbis( ogg, audio, file ) );
        AUDIO::CAudioBuffer reference;
        ASSERT_TRUE( DecodeFile( ogg, file, reference ) );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( ogg, file, source ) );
        AUDIO::CAudioBuffer played;
        UInt64 underruns = 1;
        ASSERT_TRUE( PlayLooped( source, 3, played, underruns ) );
        ASSERT_TRUE( 3 * 30001 == played.GetFrameCount() );
        ASSERT_TRUE( 0 == underruns );
        for ( UInt32 loop=0; loop<3; ++loop )
            ASSERT_TRUE( 0 == memcmp( played.GetDataPtr( loop * 30001 ), reference.GetDataPtr(), reference.GetSizeInBytes() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Vorbis surround channels are in WAVE order: each tone is on its channel, the speaker mask is right" )
    try
    {
        const UInt32 masks[] = { 0, 0, 0, 0x007, 0x033, 0x037, 0x03F, 0x70F, 0x63F };
        for ( UInt32 channels=1; channels<=8; ++channels )
        {
            const AUDIO::CAudioFormat format( 48000, channels, AUDIO::SAMPLETYPE_INT16 );
            const AUDIO::CAudioBuffer audio = MakeChannelTones( format, 30000, 0.3 );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeVorbis( ogg, audio, file ) );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( DecodeFile( ogg, file, decoded ) );
            ASSERT_TRUE( 30000 == decoded.GetFrameCount() );
            for ( UInt32 c=0; c<channels; ++c )
            {
                ASSERT_TRUE( c == WhereIsTone( decoded, c, channels ) );
                const Float64 expected = ToneAmplitude( audio, c, ChannelTone( c, channels ), 5000, 16000 );
                ASSERT_TRUE( fabs( ToDecibel( ToneAmplitude( decoded, c, ChannelTone( c, channels ), 5000, 16000 ) / expected ) ) < 2.0 );
            }
            const AUDIO::CAudioFormat expectedFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32, masks[ channels ] );
            ASSERT_TRUE( decoded.GetFormat() == expectedFormat );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Chained Vorbis streams play as one, one with another format ends the source before it" )
    try
    {
        const AUDIO::CAudioFormat mono( 44100, 1, AUDIO::SAMPLETYPE_INT16 );
        const AUDIO::CAudioBuffer low = MakeTones( mono, 44100, 440.0, 0.5 );
        const AUDIO::CAudioBuffer high = MakeTones( mono, 30000, 880.0, 0.5 );
        CORE::CDynamicBuffer a, b, c, chained;
        ASSERT_TRUE( EncodeVorbis( ogg, low, a, "0.8", "11" ) );
        ASSERT_TRUE( EncodeVorbis( ogg, high, b, "0.8", "22" ) );
        ASSERT_TRUE( EncodeVorbis( ogg, low, c, "0.8", "33" ) );

        // two links of the same format: one stream of the length of both
        AppendFile( chained, a );
        AppendFile( chained, b );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( ogg, chained, source ) );
        ASSERT_TRUE( 44100 + 30000 == source->GetLengthInFrames() );
        AUDIO::CAudioBuffer decoded;
        ASSERT_TRUE( ReadAll( *source, decoded ) );
        ASSERT_TRUE( 74100 == decoded.GetFrameCount() );
        ASSERT_TRUE( ToneAmplitude( decoded, 0, 440.0, 10000, 8000 ) > 0.45 && ToneAmplitude( decoded, 0, 880.0, 10000, 8000 ) < 0.02 );
        ASSERT_TRUE( ToneAmplitude( decoded, 0, 880.0, 50000, 8000 ) > 0.45 && ToneAmplitude( decoded, 0, 440.0, 50000, 8000 ) < 0.02 );

        // and seeks over the link boundary, exactly as reading on does
        CORE::CDynamicBuffer chunk( 1000 * 4, true );
        const UInt32 targets[] = { 43500, 44099, 44100, 44101, 60000, 74099, 0 };
        for ( UInt32 i=0; i<sizeof( targets ) / sizeof( targets[ 0 ] ); ++i )
        {
            ASSERT_TRUE( source->SeekToFrame( targets[ i ] ) );
            const UInt32 expected = 74100 - targets[ i ] < 1000 ? 74100 - targets[ i ] : 1000;
            ASSERT_TRUE( expected == source->ReadFrames( chunk.GetBufferPtr(), 1000 ) );
            ASSERT_TRUE( 0 == memcmp( chunk.GetConstBufferPtr(), decoded.GetDataPtr( targets[ i ] ), (size_t) expected * 4 ) );
        }

        // three links
        AppendFile( chained, c );
        ASSERT_TRUE( OpenSource( ogg, chained, source ) );
        ASSERT_TRUE( 44100 + 30000 + 44100 == source->GetLengthInFrames() );
        ASSERT_TRUE( ReadAll( *source, decoded ) );
        ASSERT_TRUE( 118200 == decoded.GetFrameCount() );

        // a link with another format: the source is the links before it
        CORE::CDynamicBuffer other, mixed;
        ASSERT_TRUE( EncodeVorbis( ogg, MakeTones( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), 20000, 500.0, 0.5 ), other, "0.8", "44" ) );
        AppendFile( mixed, a );
        AppendFile( mixed, other );
        AppendFile( mixed, c );
        ASSERT_TRUE( OpenSource( ogg, mixed, source ) );
        ASSERT_TRUE( source->GetFormat() == AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( 44100 == source->GetLengthInFrames() );
        ASSERT_TRUE( ReadAll( *source, decoded ) );
        ASSERT_TRUE( 44100 == decoded.GetFrameCount() );
        ASSERT_TRUE( source->IsEndOfStream() );
        ASSERT_TRUE( source->SeekToFrame( 44100 ) );
        ASSERT_FALSE( source->SeekToFrame( 44101 ) );
        ASSERT_TRUE( source->SeekToFrame( 44000 ) );
        ASSERT_TRUE( 100 == source->ReadFrames( chunk.GetBufferPtr(), 1000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Vorbis comments reach the stream and the metadata with lower case names, loop points included" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeTones( AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_INT16 ), 20000, 500.0, 0.5 );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( EncodeVorbis( ogg, audio, file, "0.5", GUCEF_NULL,
            "TITLE=Test Tune\nArtist=The Band\nLOOPSTART=1000\nLoopLength=2500\nLOOPEND=3500\nNOEQUALSSIGN\n=nameless\nTITLE=Second title\nCOMMENT=caf\xC3\xA9" ) );

        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( OpenSource( ogg, file, source ) );
        CORE::CValueList tags;
        ASSERT_TRUE( source->GetMetaData( tags ) );
        ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Test Tune" );
        ASSERT_TRUE( tags.GetValueAlways( "artist" ).AsString() == "The Band" );
        ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "1000" );
        ASSERT_TRUE( tags.GetValueAlways( "looplength" ).AsString() == "2500" );
        ASSERT_TRUE( tags.GetValueAlways( "loopend" ).AsString() == "3500" );
        ASSERT_TRUE( tags.GetValueAlways( "comment" ).AsString() == "caf\xC3\xA9" );

        // asking does not disturb the stream
        Float32 frames[ 100 ];
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );
        ASSERT_TRUE( source->GetMetaData( tags ) );
        ASSERT_TRUE( 100 == source->GetPositionInFrames() );

        CORE::CValueList metaData;
        CORE::CDynamicBufferAccess input( file );
        ASSERT_TRUE( ogg->DecodeMetaData( input, metaData ) );
        ASSERT_TRUE( metaData.GetValueAlways( "looplength" ).AsString() == "2500" );
        ASSERT_TRUE( metaData.GetValueAlways( "frames" ).AsString() == "20000" );
        ASSERT_TRUE( metaData.GetValueAlways( "sampleRate" ).AsString() == "44100" );

        // through a stream, the way music plays
        AUDIO::CAudioStream stream( source, 200, 1, false );
        CORE::CValueList streamTags;
        ASSERT_TRUE( stream.GetMetaData( streamTags ) );
        ASSERT_TRUE( streamTags.GetValueAlways( "loopstart" ).AsString() == "1000" );

        // a file without comments has only what the library adds itself
        CORE::CDynamicBuffer plain;
        ASSERT_TRUE( EncodeVorbis( ogg, audio, plain ) );
        AUDIO::TAudioSourcePtr plainSource;
        ASSERT_TRUE( OpenSource( ogg, plain, plainSource ) );
        CORE::CValueList none;
        plainSource->GetMetaData( none );
        ASSERT_FALSE( none.HasKey( "loopstart" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: The probe knows Vorbis and Opus streams by their first packet, and Ogg FLAC is turned down" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeTones( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_INT16 ), 20000, 500.0, 0.5 );
        CORE::CDynamicBuffer vorbisFile, opusFile;
        ASSERT_TRUE( EncodeVorbis( ogg, audio, vorbisFile ) );

        UInt32 confidence = 0;
        {
            CORE::CDynamicBufferAccess input( vorbisFile );
            ASSERT_TRUE( ogg->Probe( input, confidence ) );
            ASSERT_TRUE( 100 == confidence );
            ASSERT_TRUE( 0 == input.Tell() );
            TCodecPtr found;
            ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( input, found, &confidence ) );
            ASSERT_TRUE( 100 == confidence );
            ASSERT_TRUE( found->GetType() == "ogg" || found->GetType() == "oga" || found->GetType() == "opus" );
            // the codec found is one that can play it, whatever the name of the file
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( found->OpenSource( input, source ) );
        }

        if ( hasOpus )
        {
            ASSERT_TRUE( EncodeOpus( opus, audio, opusFile ) );
            CORE::CDynamicBufferAccess input( opusFile );
            ASSERT_TRUE( opus->Probe( input, confidence ) );
            ASSERT_TRUE( 100 == confidence );
            // the same plugin: a .ogg codec plays Opus and an .opus codec plays Vorbis
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( ogg, opusFile, source ) );
            ASSERT_TRUE( 48000 == source->GetFormat().GetSampleRate() );
            ASSERT_TRUE( OpenSource( opus, vorbisFile, source ) );
        }

        // Ogg FLAC and Ogg with something else in it: recognised, not played
        CORE::CDynamicBuffer flacInOgg, theora, junk;
        const UInt8 flacPacket[ 20 ] = { 0x7F, 'F', 'L', 'A', 'C', 1, 0, 0, 1, 'f', 'L', 'a', 'C', 0, 0, 0, 34, 0, 0, 0 };
        MakeOggStart( flacInOgg, flacPacket, sizeof( flacPacket ) );
        CORE::CDynamicBufferAccess flacInput( flacInOgg );
        ASSERT_TRUE( ogg->Probe( flacInput, confidence ) );
        ASSERT_TRUE( 10 == confidence );
        AUDIO::TAudioSourcePtr none;
        ASSERT_FALSE( OpenSource( ogg, flacInOgg, none ) );
        const UInt8 theoraPacket[ 20 ] = { 0x80, 't', 'h', 'e', 'o', 'r', 'a', 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        MakeOggStart( theora, theoraPacket, sizeof( theoraPacket ) );
        CORE::CDynamicBufferAccess theoraInput( theora );
        ASSERT_TRUE( ogg->Probe( theoraInput, confidence ) );
        ASSERT_TRUE( 5 == confidence );
        ASSERT_FALSE( OpenSource( ogg, theora, none ) );

        // nothing Ogg
        CRandom random( 12 );
        CORE::CDynamicBuffer noise( 300, true );
        for ( UInt32 i=0; i<300; ++i )
            ( (UInt8*) noise.GetBufferPtr() )[ i ] = (UInt8) random.Next();
        CORE::CDynamicBufferAccess noiseInput( noise );
        ASSERT_TRUE( ogg->Probe( noiseInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );
        TCodecPtr found;
        ASSERT_FALSE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( noiseInput, found ) );
        ASSERT_FALSE( OpenSource( ogg, noise, none ) );
        // an Ogg file that starts half way is not the start of a stream
        CORE::CDynamicBuffer half( (const char*) vorbisFile.GetConstBufferPtr() + 100, 3000, true );
        ASSERT_FALSE( OpenSource( ogg, half, none ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Truncated and corrupted Vorbis files never crash or hang the decoder" )
    try
    {
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( EncodeVorbis( ogg, MakeTones( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 2400, 300.0, 0.5 ), file, "0.0", GUCEF_NULL, "TITLE=Sweep\nLOOPSTART=5" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Vorbis sweep file: " << file.GetDataSize() << " bytes" );
        ASSERT_TRUE( SweepDamage( ogg, file, 1, 1000, 100000000, 41 ) == 0 );

        // a chained one, where the links are damaged
        CORE::CDynamicBuffer second, chained;
        ASSERT_TRUE( EncodeVorbis( ogg, MakeTones( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 1200, 500.0, 0.5 ), second, "0.0", "77" ) );
        AppendFile( chained, file );
        AppendFile( chained, second );
        ASSERT_TRUE( SweepDamage( ogg, chained, 11, 500, 100000000, 42 ) == 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Two Vorbis streams of the plugin decode at the same time, per source" )
    try
    {
        const AUDIO::CAudioBuffer audio = MakeNoise( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), 3 * 44100, 0.3f, 6 );
        CORE::CDynamicBuffer file;
        ASSERT_TRUE( EncodeVorbis( ogg, audio, file, "0.5" ) );
        AUDIO::CAudioBuffer reference;
        ASSERT_TRUE( DecodeFile( ogg, file, reference ) );
        Int32 maxOverlap = 0;
        ASSERT_TRUE( DecodeTwoAtOnce( ogg, file, reference, maxOverlap ) );
        ASSERT_TRUE( 2 == maxOverlap );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // ---- Opus ----

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Opus round trips: exact length, no shift, level within half a decibel and a good SNR" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            const UInt32 channelCounts[] = { 1, 2 };
            for ( UInt32 i=0; i<2; ++i )
            {
                const AUDIO::CAudioFormat format( 48000, channelCounts[ i ], AUDIO::SAMPLETYPE_INT16 );
                const AUDIO::CAudioBuffer audio = MakeTones( format, 48000 * 2 + 321, 1000.0, 0.5 );
                CORE::CDynamicBuffer file;
                ASSERT_TRUE( EncodeOpus( opus, audio, file ) );

                AUDIO::TAudioSourcePtr source;
                ASSERT_TRUE( OpenSource( opus, file, source ) );
                ASSERT_TRUE( source->GetFormat() == AUDIO::CAudioFormat( 48000, channelCounts[ i ], AUDIO::SAMPLETYPE_FLOAT32 ) );
                ASSERT_TRUE( audio.GetFrameCount() == source->GetLengthInFrames() );
                AUDIO::CAudioBuffer decoded;
                ASSERT_TRUE( ReadAll( *source, decoded ) );
                Float64 snr = 0.0;
                const bool isOk = LossyMatches( audio, decoded, 1000.0, 30.0, snr );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Opus " << channelCounts[ i ] << " ch: SNR " << snr << " dB, " << file.GetDataSize() << " bytes" );
                ASSERT_TRUE( isOk );

                const AUDIO::CAudioBuffer noise = MakeNoise( format, 20000, 0.3f, 9 );
                CORE::CDynamicBuffer noiseFile;
                AUDIO::CAudioBuffer noiseDecoded;
                ASSERT_TRUE( EncodeOpus( opus, noise, noiseFile ) );
                ASSERT_TRUE( DecodeFile( opus, noiseFile, noiseDecoded ) );
                ASSERT_TRUE( 20000 == noiseDecoded.GetFrameCount() );
                ASSERT_TRUE( IsNotShifted( noise, noiseDecoded ) );
            }

            // Opus is 48 kHz: other rates are not written
            CORE::CDynamicBuffer rejected;
            ASSERT_FALSE( EncodeOpus( opus, MakeTones( AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_INT16 ), 4410, 500.0, 0.5 ), rejected ) );
            ASSERT_FALSE( EncodeOpus( opus, MakeTones( AUDIO::CAudioFormat( 48000, 9, AUDIO::SAMPLETYPE_INT16 ), 4800, 500.0, 0.5 ), rejected ) );

            // the type decides which codec the encoder writes, unless the option says otherwise
            CORE::CDynamicBuffer viaOgg, viaOpus;
            const AUDIO::CAudioBuffer short48 = MakeTones( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_INT16 ), 9600, 500.0, 0.5 );
            CORE::CValueList asOpus;
            asOpus.Set( "codec", "opus" );
            ASSERT_TRUE( Encode( ogg, short48, viaOgg, asOpus ) );
            ASSERT_TRUE( EncodeOpus( opus, short48, viaOpus, "64000" ) );
            ASSERT_TRUE( 0 == memcmp( (const char*) viaOgg.GetConstBufferPtr() + 28, "OpusHead", 8 ) );
            CORE::CValueList asVorbis;
            asVorbis.Set( "codec", "vorbis" );
            CORE::CDynamicBuffer vorbis;
            ASSERT_TRUE( Encode( opus, short48, vorbis, asVorbis ) );
            ASSERT_TRUE( 0 == memcmp( (const char*) vorbis.GetConstBufferPtr() + 28, "\x01vorbis", 7 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Opus seeking lands on the right frame, and within -60 dB of reading on once the decoder has settled, for 50 random places" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            // tonal music: Opus fills what it does not code with noise from a generator that runs on, so after a seek
            // that part is another noise, and white noise itself would be nothing but that
            const AUDIO::CAudioFormat format( 48000, 2, AUDIO::SAMPLETYPE_INT16 );
            const UInt32 frames = 3 * 48000 + 777;
            AUDIO::CAudioBuffer audio( format, frames );
            for ( UInt32 f=0; f<frames; ++f )
                for ( UInt32 c=0; c<2; ++c )
                {
                    const Float64 t = (Float64) f / 48000.0;
                    const Float64 vibrato = 1.0 + 0.003 * sin( 6.0 * t );
                    const Float64 value = 0.25 * sin( 6.283185307 * ( 220.0 + 110.0 * c ) * vibrato * t ) + 0.15 * sin( 6.283185307 * ( 1234.5 + 100.0 * c ) * t ) * ( 0.6 + 0.4 * sin( 1.7 * t ) ) + 0.1 * sin( 6.283185307 * 3150.0 * t + 2.0 * sin( 3.1 * t ) );
                    audio.SetSampleFromFloat( f, c, (Float32) value );
                }
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeOpus( opus, audio, file, "128000" ) );

            AUDIO::CAudioBuffer reference;
            ASSERT_TRUE( DecodeFile( opus, file, reference ) );
            const UInt32 length = reference.GetFrameCount();
            ASSERT_TRUE( audio.GetFrameCount() == length );

            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( opus, file, source ) );
            ASSERT_TRUE( source->IsSeekable() );
            CRandom random( 99 );
            CORE::CDynamicBuffer chunk( 1024 * 8, true );
            const UInt32 convergence = 24000;                       // half a second: where the decoder has found its state again
            Float64 worstFirst = -1000.0, worstConverged = -1000.0;
            for ( UInt32 i=0; i<54; ++i )
            {
                UInt32 target = random.Below( length - convergence - 1024 );
                if ( 0 == i ) target = 0;
                if ( 1 == i ) target = 960;
                if ( 2 == i ) target = 959;
                if ( 3 == i ) target = length - 1;
                ASSERT_TRUE( source->SeekToFrame( target ) );
                const UInt32 expected = length - target < 1024 ? length - target : 1024;
                ASSERT_TRUE( expected == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );

                // the first frames after a seek are the decoder settling (the error there is what it is, up to the size of the signal); at the start of the stream, where it has nothing to settle from, it is exact
                Float64 error = 0.0, energy = 0.0;
                const AUDIO::CAudioBuffer got( reference.GetFormat(), chunk.GetConstBufferPtr(), expected );
                for ( UInt32 f=0; f<expected; ++f )
                    for ( UInt32 c=0; c<2; ++c )
                    {
                        const Float64 r = reference.GetSampleAsFloat( target + f, c );
                        const Float64 g = got.GetSampleAsFloat( f, c );
                        error += ( r - g ) * ( r - g );
                        energy += r * r;
                    }
                const Float64 first = 10.0 * log10( ( error + 1.0e-20 ) / ( energy + 1.0e-20 ) );
                worstFirst = first > worstFirst ? first : worstFirst;
                if ( 0 == target || 960 == target || 959 == target )
                    ASSERT_TRUE( first < -100.0 );

                // after that the decoder has the state it would have had reading on
                if ( target + convergence + 1024 <= length )
                {
                    CORE::CDynamicBuffer skip( convergence * 8, true );
                    ASSERT_TRUE( convergence == source->ReadFrames( skip.GetBufferPtr(), convergence ) );
                    ASSERT_TRUE( 1024 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
                    const AUDIO::CAudioBuffer later( reference.GetFormat(), chunk.GetConstBufferPtr(), 1024 );
                    error = 0.0;
                    energy = 0.0;
                    for ( UInt32 f=0; f<1024; ++f )
                        for ( UInt32 c=0; c<2; ++c )
                        {
                            const Float64 r = reference.GetSampleAsFloat( target + convergence + f + 1024, c );
                            const Float64 g = later.GetSampleAsFloat( f, c );
                            error += ( r - g ) * ( r - g );
                            energy += r * r;
                        }
                    const Float64 converged = 10.0 * log10( ( error + 1.0e-20 ) / ( energy + 1.0e-20 ) );
                    worstConverged = converged > worstConverged ? converged : worstConverged;
                    ASSERT_TRUE( converged < -60.0 );
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Opus seek, worst error against reading on: " << worstFirst << " dB at the target, " << worstConverged << " dB half a second later" );
            ASSERT_TRUE( source->SeekToFrame( length ) );
            ASSERT_TRUE( 0 == source->ReadFrames( chunk.GetBufferPtr(), 1024 ) );
            ASSERT_FALSE( source->SeekToFrame( length + 1 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: An Opus stream that loops three times has no gap at the seams" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            const AUDIO::CAudioBuffer audio = MakeTones( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), 30001, 600.0, 0.4 );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeOpus( opus, audio, file ) );
            AUDIO::CAudioBuffer reference;
            ASSERT_TRUE( DecodeFile( opus, file, reference ) );

            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( opus, file, source ) );
            AUDIO::CAudioBuffer played;
            UInt64 underruns = 1;
            ASSERT_TRUE( PlayLooped( source, 3, played, underruns ) );
            ASSERT_TRUE( 3 * 30001 == played.GetFrameCount() );
            ASSERT_TRUE( 0 == underruns );
            for ( UInt32 loop=0; loop<3; ++loop )
                ASSERT_TRUE( SameFrames( reference, 0, played, loop * 30001, 30001, 1.0e-4f ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Opus surround channels are in WAVE order: each tone is on its channel, the speaker mask is right" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            const UInt32 masks[] = { 0, 0, 0, 0x007, 0x033, 0x037, 0x03F, 0x70F, 0x63F };
            for ( UInt32 channels=1; channels<=8; ++channels )
            {
                const AUDIO::CAudioFormat format( 48000, channels, AUDIO::SAMPLETYPE_INT16 );
                const AUDIO::CAudioBuffer audio = MakeChannelTones( format, 30000, 0.3 );
                CORE::CDynamicBuffer file;
                ASSERT_TRUE( EncodeOpus( opus, audio, file, "320000" ) );
                AUDIO::CAudioBuffer decoded;
                ASSERT_TRUE( DecodeFile( opus, file, decoded ) );
                ASSERT_TRUE( 30000 == decoded.GetFrameCount() );
                for ( UInt32 c=0; c<channels; ++c )
                    ASSERT_TRUE( c == WhereIsTone( decoded, c, channels ) );
                const AUDIO::CAudioFormat expectedFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32, masks[ channels ] );
                ASSERT_TRUE( decoded.GetFormat() == expectedFormat );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Chained Opus streams play as one, one with another channel count ends the source before it" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            const AUDIO::CAudioFormat mono( 48000, 1, AUDIO::SAMPLETYPE_INT16 );
            CORE::CDynamicBuffer a, b, stereo, chained;
            ASSERT_TRUE( EncodeOpus( opus, MakeTones( mono, 48000, 440.0, 0.5 ), a, "64000", "5" ) );
            ASSERT_TRUE( EncodeOpus( opus, MakeTones( mono, 30000, 880.0, 0.5 ), b, "64000", "6" ) );
            ASSERT_TRUE( EncodeOpus( opus, MakeTones( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), 20000, 500.0, 0.5 ), stereo, "64000", "7" ) );

            AppendFile( chained, a );
            AppendFile( chained, b );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( opus, chained, source ) );
            ASSERT_TRUE( 78000 == source->GetLengthInFrames() );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( ReadAll( *source, decoded ) );
            ASSERT_TRUE( 78000 == decoded.GetFrameCount() );
            ASSERT_TRUE( ToneAmplitude( decoded, 0, 440.0, 10000, 8000 ) > 0.4 && ToneAmplitude( decoded, 0, 880.0, 10000, 8000 ) < 0.03 );
            ASSERT_TRUE( ToneAmplitude( decoded, 0, 880.0, 55000, 8000 ) > 0.4 && ToneAmplitude( decoded, 0, 440.0, 55000, 8000 ) < 0.03 );

            CORE::CDynamicBuffer mixed;
            AppendFile( mixed, a );
            AppendFile( mixed, stereo );
            AppendFile( mixed, b );
            ASSERT_TRUE( OpenSource( opus, mixed, source ) );
            ASSERT_TRUE( 1 == source->GetFormat().GetChannelCount() );
            ASSERT_TRUE( 48000 == source->GetLengthInFrames() );
            ASSERT_TRUE( ReadAll( *source, decoded ) );
            ASSERT_TRUE( 48000 == decoded.GetFrameCount() );
            ASSERT_FALSE( source->SeekToFrame( 48001 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Opus tags reach the stream and the metadata with lower case names, loop points included" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            const AUDIO::CAudioBuffer audio = MakeTones( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_INT16 ), 20000, 500.0, 0.5 );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeOpus( opus, audio, file, "64000", GUCEF_NULL, "TITLE=Voice\nLOOPSTART=480\nLoopLength=9600\nPERFORMER=\xC3\x9Cnique" ) );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( OpenSource( opus, file, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Voice" );
            ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "480" );
            ASSERT_TRUE( tags.GetValueAlways( "looplength" ).AsString() == "9600" );
            ASSERT_TRUE( tags.GetValueAlways( "performer" ).AsString() == "\xC3\x9Cnique" );

            CORE::CValueList metaData;
            CORE::CDynamicBufferAccess input( file );
            ASSERT_TRUE( opus->DecodeMetaData( input, metaData ) );
            ASSERT_TRUE( metaData.GetValueAlways( "loopstart" ).AsString() == "480" );
            ASSERT_TRUE( metaData.GetValueAlways( "frames" ).AsString() == "20000" );
            ASSERT_TRUE( metaData.GetValueAlways( "sampleRate" ).AsString() == "48000" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Truncated and corrupted Opus files never crash or hang the decoder" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeOpus( opus, MakeTones( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), 2400, 300.0, 0.5 ), file, "32000", GUCEF_NULL, "TITLE=Sweep\nLOOPSTART=5" ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Opus sweep file: " << file.GetDataSize() << " bytes" );
            ASSERT_TRUE( SweepDamage( opus, file, 1, 1000, 100000000, 51 ) == 0 );

            // a surround one, with a header that has the channel mapping table
            CORE::CDynamicBuffer surround;
            ASSERT_TRUE( EncodeOpus( opus, MakeTones( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_INT16 ), 1920, 300.0, 0.3 ), surround, "96000" ) );
            ASSERT_TRUE( SweepDamage( opus, surround, 3, 500, 100000000, 52 ) == 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Two Opus streams of the plugin decode at the same time, per source" )
    {
        GUCEF_TESTFW_SKIP_IF( !hasOpus, "this build of the plugin has no Opus" );
        try
        {
            const AUDIO::CAudioBuffer audio = MakeNoise( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), 3 * 48000, 0.3f, 6 );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( EncodeOpus( opus, audio, file, "128000" ) );
            AUDIO::CAudioBuffer reference;
            ASSERT_TRUE( DecodeFile( opus, file, reference ) );
            Int32 maxOverlap = 0;
            ASSERT_TRUE( DecodeTwoAtOnce( opus, file, reference, maxOverlap ) );
            ASSERT_TRUE( 2 == maxOverlap );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Writes Vorbis and Opus files of generated signals into GUCEF_TEST_AUDIO_WRITE_SIGNALS (opt in)" )
    {
        const CORE::CString directory( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_WRITE_SIGNALS" ).c_str() );
        GUCEF_TESTFW_SKIP_IF( directory.IsNULLOrEmpty(), "GUCEF_TEST_AUDIO_WRITE_SIGNALS is not set" );
        try
        {
            // for trying the files in other players and decoders: the same tones as the channel tests use, with tags
            const CORE::CString target = CORE::CombinePath( directory, "ogg" );
            ASSERT_TRUE( CORE::CreatePathDirectories( CORE::CombinePath( target, "file" ) ) );
            const char* tags = "TITLE=Generated tones\nLOOPSTART=24000\nLOOPLENGTH=48000";
            const UInt32 channelCounts[] = { 1, 2, 6 };
            const char* labels[] = { "mono", "stereo", "six-channel" };
            for ( UInt32 i=0; i<3; ++i )
            {
                const AUDIO::CAudioBuffer audio = MakeChannelTones( AUDIO::CAudioFormat( 48000, channelCounts[ i ], AUDIO::SAMPLETYPE_INT16 ), 96000, 0.3 );
                CORE::CDynamicBuffer file;
                ASSERT_TRUE( EncodeVorbis( ogg, audio, file, "0.8", GUCEF_NULL, tags ) );
                ASSERT_TRUE( file.WriteContentToFile( CORE::CombinePath( target, CORE::CString( "vorbis-" ) + labels[ i ] + ".ogg" ), 0, true ) );
                if ( hasOpus )
                {
                    ASSERT_TRUE( EncodeOpus( opus, audio, file, "192000", GUCEF_NULL, tags ) );
                    ASSERT_TRUE( file.WriteContentToFile( CORE::CombinePath( target, CORE::CString( "opus-" ) + labels[ i ] + ".opus" ), 0, true ) );
                }
            }
            const AUDIO::CAudioFormat mono( 48000, 1, AUDIO::SAMPLETYPE_INT16 );
            CORE::CDynamicBuffer a, b, chained;
            ASSERT_TRUE( EncodeVorbis( ogg, MakeTones( mono, 48000, 440.0, 0.4 ), a, "0.8", "1" ) );
            ASSERT_TRUE( EncodeVorbis( ogg, MakeTones( mono, 24000, 880.0, 0.4 ), b, "0.8", "2" ) );
            AppendFile( chained, a );
            AppendFile( chained, b );
            ASSERT_TRUE( chained.WriteContentToFile( CORE::CombinePath( target, "vorbis-chained.ogg" ), 0, true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecOgg: Unloading the plugin takes the types away" )
    try
    {
        ogg = TCodecPtr();
        oga = TCodecPtr();
        opus = TCodecPtr();
        ASSERT_TRUE( UnloadPlugin( PluginGroup, "ogg" ) );
        ASSERT_TRUE( FindCodec( "oga" ).IsNULL() && FindCodec( "opus" ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
