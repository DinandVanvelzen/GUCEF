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

#ifndef GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#include "gucefAUDIO_CAudioCodecPluginManager.h"
#define GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#endif /* GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioCodecPlugin.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* PluginGroup = "audiocodecplugin";

bool
LoadAuPlugin( void )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/audiocodecpluginAU_d";
    const CORE::CString second = "$MODULEDIR$/audiocodecpluginAU";
    #else
    const CORE::CString first = "$MODULEDIR$/audiocodecpluginAU";
    const CORE::CString second = "$MODULEDIR$/audiocodecpluginAU_d";
    #endif

    AUDIO::CAudioCodecRegistry::TAudioCodecPtr codec;
    if ( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodec( "au", codec ) )
        return true;
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, PluginGroup, true ) || pluginControl.AddPluginFromDir( second, PluginGroup, true );
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioCodecRegistry::TAudioCodecPtr
FindCodec( const char* type )
{
    AUDIO::CAudioCodecRegistry::TAudioCodecPtr codec;
    AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodec( type, codec );
    return codec;
}

/*-------------------------------------------------------------------------*/

void
AppendBE32( CORE::CDynamicBuffer& buffer, UInt32 value )
{
    UInt8 bytes[ 4 ] = { (UInt8) ( value >> 24 ), (UInt8) ( value >> 16 ), (UInt8) ( value >> 8 ), (UInt8) value };
    buffer.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

/** An .au file made by hand, so the decoder is checked against the format and not against our own encoder */
void
MakeAuFile( CORE::CDynamicBuffer& file, UInt32 encoding, UInt32 rate, UInt32 channels, const void* data, UInt32 dataSize, const char* annotation, bool sizeKnown = true )
{
    UInt32 annotationSize = GUCEF_NULL != annotation ? ( (UInt32) strlen( annotation ) + 4 ) & ~3U : 4;
    file.Clear();
    AppendBE32( file, 0x2E736E64 );
    AppendBE32( file, 24 + annotationSize );
    AppendBE32( file, sizeKnown ? dataSize : 0xFFFFFFFF );
    AppendBE32( file, encoding );
    AppendBE32( file, rate );
    AppendBE32( file, channels );
    char padding[ 64 ];
    memset( padding, 0, sizeof( padding ) );
    if ( GUCEF_NULL != annotation )
        memcpy( padding, annotation, strlen( annotation ) );
    file.Append( padding, annotationSize );
    file.Append( data, dataSize );
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioBuffer
MakeRamp( const AUDIO::CAudioFormat& format, UInt32 frames )
{
    AUDIO::CAudioBuffer audio( format, frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
            audio.SetSampleFromFloat( f, c, (Float32) sin( 0.01 * f + c ) * 0.8f );
    return audio;
}

/*-------------------------------------------------------------------------*/

bool
EncodeDecode( AUDIO::CIAudioCodec& codec, const AUDIO::CAudioBuffer& audio, AUDIO::CAudioBuffer& decoded, const CORE::CValueList& options = CORE::CValueList() )
{
    CORE::CDynamicBuffer file;
    CORE::CDynamicBufferAccess output( file );
    if ( !codec.Encode( audio, output, options ) )
        return false;
    CORE::CDynamicBufferAccess input( file );
    return codec.DecodeAll( input, decoded );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioCodecPluginTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING audio codec C plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.CodecPlugin" );

    const bool loaded = LoadAuPlugin();

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: The .au plugin loads through the C interface and registers its codec types" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !loaded )
        GUCEF_TESTFW_SKIP( "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( loaded );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr snd = FindCodec( ".SND" );
        ASSERT_FALSE( au.IsNULL() );
        ASSERT_FALSE( snd.IsNULL() );
        ASSERT_TRUE( au->GetType() == "au" );
        ASSERT_TRUE( au->CanEncode() );

        AUDIO::CAudioCodecRegistry::TAudioCodecPtr byName;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForFilename( "music/theme.au", byName ) );

        AUDIO::CAudioCodecPluginManager::TAudioCodecCPluginPtr plugin = AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "AU" );
        ASSERT_FALSE( plugin.IsNULL() );
        ASSERT_TRUE( plugin->IsLinked() );
        ASSERT_TRUE( plugin->GetName().HasSubstr( ".au" ) >= 0 );
        ASSERT_TRUE( 2 == plugin->GetRegisteredCodecTypes().size() );
        ASSERT_TRUE( 1 == plugin->GetVersion().minor );

        // the built in codecs keep their types
        ASSERT_TRUE( FindCodec( "wav" )->GetType() == "wav" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !loaded )
        return;

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: Decodes mu-law, A-law and big endian 16 bit files written by hand" )
    try
    {
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );

        // G.711 reference points: silence, the extremes and the smallest steps
        const UInt8 mulaw[ 4 ] = { 0xFF, 0x7F, 0x00, 0x80 };
        CORE::CDynamicBuffer file;
        MakeAuFile( file, 1, 8000, 1, mulaw, sizeof( mulaw ), GUCEF_NULL );
        CORE::CDynamicBufferAccess mulawInput( file );
        AUDIO::CAudioBuffer decoded;
        ASSERT_TRUE( au->DecodeAll( mulawInput, decoded ) );
        ASSERT_TRUE( decoded.GetFormat() == AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( 4 == decoded.GetFrameCount() );
        const Int16* samples = (const Int16*) decoded.GetDataPtr();
        ASSERT_TRUE( 0 == samples[ 0 ] && 0 == samples[ 1 ] );
        ASSERT_TRUE( -32124 == samples[ 2 ] && 32124 == samples[ 3 ] );

        const UInt8 alaw[ 4 ] = { 0xD5, 0x55, 0xAA, 0x2A };
        MakeAuFile( file, 27, 8000, 1, alaw, sizeof( alaw ), GUCEF_NULL );
        CORE::CDynamicBufferAccess alawInput( file );
        ASSERT_TRUE( au->DecodeAll( alawInput, decoded ) );
        samples = (const Int16*) decoded.GetDataPtr();
        ASSERT_TRUE( 8 == samples[ 0 ] && -8 == samples[ 1 ] );
        ASSERT_TRUE( 32256 == samples[ 2 ] && -32256 == samples[ 3 ] );

        // stereo, big endian 16 bit with a title, length from the header
        const UInt8 linear[ 8 ] = { 0x12, 0x34, 0xFF, 0xFE, 0x80, 0x00, 0x7F, 0xFF };
        MakeAuFile( file, 3, 44100, 2, linear, sizeof( linear ), "Theme of the test" );
        CORE::CDynamicBufferAccess linearInput( file );
        CORE::CValueList metaData;
        ASSERT_TRUE( au->DecodeMetaData( linearInput, metaData ) );
        ASSERT_TRUE( metaData.GetValueAlways( "sampleRate" ).AsString() == "44100" );
        ASSERT_TRUE( metaData.GetValueAlways( "channels" ).AsString() == "2" );
        ASSERT_TRUE( metaData.GetValueAlways( "frames" ).AsString() == "2" );
        ASSERT_TRUE( metaData.GetValueAlways( "comment" ).AsString() == "Theme of the test" );
        ASSERT_TRUE( metaData.GetValueAlways( "encoding" ).AsString() == "linear" );

        linearInput.Setpos( 0 );
        ASSERT_TRUE( au->DecodeAll( linearInput, decoded ) );
        ASSERT_TRUE( 2 == decoded.GetFrameCount() && 2 == decoded.GetFormat().GetChannelCount() );
        samples = (const Int16*) decoded.GetDataPtr();
        ASSERT_TRUE( 0x1234 == samples[ 0 ] && -2 == samples[ 1 ] && -32768 == samples[ 2 ] && 32767 == samples[ 3 ] );

        // signed 8 bit in the file is unsigned in gucefAUDIO; no size in the header: read until the data ends
        const UInt8 linear8[ 3 ] = { 0x00, 0x7F, 0x80 };
        MakeAuFile( file, 2, 11025, 1, linear8, sizeof( linear8 ), GUCEF_NULL, false );
        CORE::CDynamicBufferAccess linear8Input( file );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( au->OpenSource( linear8Input, source ) );
        ASSERT_TRUE( GUCEF_AUDIO_LENGTH_UNKNOWN == source->GetLengthInFrames() );
        ASSERT_TRUE( AUDIO::SAMPLETYPE_UINT8 == source->GetFormat().GetSampleType() );
        UInt8 bytes[ 8 ];
        ASSERT_TRUE( 3 == source->ReadFrames( bytes, 8 ) );
        ASSERT_TRUE( 128 == bytes[ 0 ] && 255 == bytes[ 1 ] && 0 == bytes[ 2 ] );
        ASSERT_TRUE( 0 == source->ReadFrames( bytes, 8 ) );
        ASSERT_TRUE( source->IsEndOfStream() );

        // not an .au file
        const char notAu[ 32 ] = "RIFF....WAVEfmt ";
        CORE::CDynamicBuffer junk( notAu, sizeof( notAu ), true );
        CORE::CDynamicBufferAccess junkInput( junk );
        AUDIO::TAudioSourcePtr none;
        ASSERT_FALSE( au->OpenSource( junkInput, none ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: Encoding and decoding again gives the same samples for every plain sample type" )
    try
    {
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );
        const AUDIO::TSampleType types[] = { AUDIO::SAMPLETYPE_UINT8, AUDIO::SAMPLETYPE_INT16, AUDIO::SAMPLETYPE_INT24,
                                             AUDIO::SAMPLETYPE_INT32, AUDIO::SAMPLETYPE_FLOAT32, AUDIO::SAMPLETYPE_FLOAT64 };
        for ( UInt32 t=0; t<sizeof( types )/sizeof( types[ 0 ] ); ++t )
        {
            // more than one chunk of the encoder, and 3 channels so frames and samples differ
            const AUDIO::CAudioBuffer audio = MakeRamp( AUDIO::CAudioFormat( 22050, 3, types[ t ] ), 5000 );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( EncodeDecode( *au, audio, decoded ) );
            ASSERT_TRUE( decoded.GetFormat() == audio.GetFormat() );
            ASSERT_TRUE( decoded.GetFrameCount() == audio.GetFrameCount() );
            ASSERT_TRUE( 0 == memcmp( decoded.GetDataPtr(), audio.GetDataPtr(), audio.GetSizeInBytes() ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: Options reach the plugin: mu-law and A-law are written within the G.711 step size" )
    try
    {
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );
        const AUDIO::CAudioBuffer audio = MakeRamp( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 4000 );
        const char* encodings[] = { "mulaw", "alaw" };
        for ( UInt32 e=0; e<2; ++e )
        {
            CORE::CValueList options;
            options.Set( "encoding", encodings[ e ] );
            CORE::CDynamicBuffer file;
            CORE::CDynamicBufferAccess output( file );
            ASSERT_TRUE( au->Encode( audio, output, options ) );
            ASSERT_TRUE( 28 + 4000 == file.GetDataSize() );                       // one byte per sample

            CORE::CDynamicBufferAccess input( file );
            AUDIO::CAudioBuffer decoded;
            ASSERT_TRUE( au->DecodeAll( input, decoded ) );
            ASSERT_TRUE( 4000 == decoded.GetFrameCount() );
            const Int16* original = (const Int16*) audio.GetDataPtr();
            const Int16* result = (const Int16*) decoded.GetDataPtr();
            for ( UInt32 i=0; i<4000; ++i )
            {
                // companding keeps about 4 significant bits: the error is at most 1/16 of the value, plus the smallest step
                const Int32 error = abs( (Int32) original[ i ] - (Int32) result[ i ] );
                if ( error > abs( (Int32) original[ i ] ) / 16 + 32 )
                {
                    ASSERT_TRUE( false );
                    break;
                }
            }
        }

        // companding needs 16 bit input, the plugin says no to anything else
        CORE::CValueList options;
        options.Set( "encoding", "mulaw" );
        CORE::CDynamicBuffer file;
        CORE::CDynamicBufferAccess output( file );
        ASSERT_FALSE( au->Encode( MakeRamp( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 10 ), output, options ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: A plugin stream seeks, and plays through the engine like any other source" )
    try
    {
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );
        const AUDIO::CAudioBuffer audio = MakeRamp( AUDIO::CAudioFormat( 22050, 2, AUDIO::SAMPLETYPE_INT16 ), 2205 );
        CORE::CDynamicBuffer file;
        CORE::CDynamicBufferAccess output( file );
        ASSERT_TRUE( au->Encode( audio, output ) );

        CORE::CIOAccess::IOAccessPtr input( GUCEF_NEW CORE::CDynamicBufferAccess( file ) );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( au->OpenSource( input, source ) );
        ASSERT_TRUE( source->IsSeekable() );
        ASSERT_TRUE( 2205 == source->GetLengthInFrames() );
        ASSERT_TRUE( source->SeekToFrame( 2000 ) );
        Int16 frames[ 2 * 300 ];
        ASSERT_TRUE( 205 == source->ReadFrames( frames, 300 ) );
        ASSERT_TRUE( 0 == memcmp( frames, audio.GetDataPtr( 2000 ), 205 * 4 ) );
        ASSERT_TRUE( source->IsEndOfStream() );
        ASSERT_FALSE( source->SeekToFrame( 2206 ) );
        ASSERT_TRUE( source->Rewind() );
        ASSERT_FALSE( source->IsEndOfStream() );

        // the null device is driven by hand: render the whole stream and compare with what went in
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 22050, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        params.GetDeviceParams().SetBufferSizeInFrames( 256 );
        params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( source );
        ASSERT_FALSE( voice.IsNULL() );
        CORE::CFutureResult started = engine.Start( voice );
        ASSERT_FALSE( started.HasNoFuture() );
        CORE::CFutureResult ended = engine.GetEndedResult( voice );
        AUDIO::CNullAudioDevice* device = ( engine.GetDevice().IsNULL() ? GUCEF_NULL : engine.GetDevice()->AsNullDevice() );
        ASSERT_TRUE( GUCEF_NULL != device );
        device->Render( 2205 + 512 );
        ASSERT_TRUE( engine.WaitFor( ended, 2000 ) );
        const AUDIO::CAudioBuffer& heard = device->GetRenderedAudio();
        ASSERT_TRUE( heard.GetFrameCount() >= 2205 );
        ASSERT_TRUE( fabs( heard.GetSampleAsFloat( 1000, 1 ) - audio.GetSampleAsFloat( 1000, 1 ) ) < 0.001f );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: Unloading the plugin with a stream open ends the stream instead of calling into the gone module" )
    try
    {
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );
        const AUDIO::CAudioBuffer audio = MakeRamp( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 1000 );
        CORE::CDynamicBuffer file;
        CORE::CDynamicBufferAccess output( file );
        ASSERT_TRUE( au->Encode( audio, output ) );
        CORE::CIOAccess::IOAccessPtr input( GUCEF_NEW CORE::CDynamicBufferAccess( file ) );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( au->OpenSource( input, source ) );
        Int16 frames[ 100 ];
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );

        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( PluginGroup ) );
        ASSERT_TRUE( FindCodec( "au" ).IsNULL() );
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "au" ).IsNULL() );

        // the codec object we still hold and the open stream are inert now
        ASSERT_TRUE( 0 == source->ReadFrames( frames, 100 ) );
        ASSERT_TRUE( source->IsEndOfStream() );
        ASSERT_FALSE( source->SeekToFrame( 0 ) );
        ASSERT_FALSE( au->CanEncode() );
        AUDIO::TAudioSourcePtr another;
        ASSERT_FALSE( au->OpenSource( input, another ) );
        source = AUDIO::TAudioSourcePtr();
        au = AUDIO::CAudioCodecRegistry::TAudioCodecPtr();

        // and it loads again
        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().LoadPluginGroup( PluginGroup ) );
        ASSERT_FALSE( FindCodec( "au" ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
