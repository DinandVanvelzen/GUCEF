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
#include <stdlib.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#include "gucefAUDIO_CAudioCodecCPluginCodec.h"
#define GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H ? */

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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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

/*-------------------------------------------------------------------------*/

/** Environment of the test plugin: how it declares its threading, see gucefAUDIO_TestCodecPlugin.c */
void
SetTestPluginThreading( const char* mode )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( "GUCEF_AUDIOCODECPLUGINAUTEST_THREADING", GUCEF_NULL != mode ? mode : "" );
    #else
    if ( GUCEF_NULL != mode )
        setenv( "GUCEF_AUDIOCODECPLUGINAUTEST_THREADING", mode, 1 );
    else
        unsetenv( "GUCEF_AUDIOCODECPLUGINAUTEST_THREADING" );
    #endif
}

/*-------------------------------------------------------------------------*/

const char* ExtensionPluginGroup = "audiocodecpluginext";

/** The test variant of the .au plugin: the optional entry points, other type names ("autest", "sndtest") */
bool
LoadExtensionPlugin( const char* threadingMode = GUCEF_NULL )
{
    SetTestPluginThreading( threadingMode );

    AUDIO::CAudioCodecRegistry::TAudioCodecPtr codec;
    if ( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodec( "autest", codec ) )
        return true;

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/gucefAUDIO_TestCodecPlugin_d";
    const CORE::CString second = "$MODULEDIR$/gucefAUDIO_TestCodecPlugin";
    #else
    const CORE::CString first = "$MODULEDIR$/gucefAUDIO_TestCodecPlugin";
    const CORE::CString second = "$MODULEDIR$/gucefAUDIO_TestCodecPlugin_d";
    #endif
    return pluginControl.AddPluginFromDir( first, ExtensionPluginGroup, true ) || pluginControl.AddPluginFromDir( second, ExtensionPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
UnloadExtensionPlugin( void )
{
    return CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( ExtensionPluginGroup ) &&
           FindCodec( "autest" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

/** Counts how many reads of the inputs that use it are in progress at the same time */
class COverlapCounter
{
    public:

    COverlapCounter( void ) : m_inside( 0 ), m_maxInside( 0 ), m_reads( 0 ) {}

    /** Called by an input around a read. 'waitForPartner' holds the first read until a second one is inside, or a while has passed */
    void Enter( bool waitForPartner )
    {
        const Int32 now = m_inside.FetchAdd( 1 ) + 1;
        m_reads.FetchAdd( 1 );
        Int32 seen = m_maxInside.Load();
        while ( now > seen && !m_maxInside.CompareExchangeWeak( seen, now ) ) {}

        if ( waitForPartner )
        {
            for ( UInt32 i=0; i<250 && m_inside.Load() < 2; ++i )
                MT::ThreadDelay( 1 );
        }
        else
            MT::ThreadDelay( 1 );
    }

    void Leave( void ) { m_inside.FetchSub( 1 ); }

    Int32 GetMaxInside( void ) const { return m_maxInside.Load(); }

    private:

    MT::CTAtomic< Int32 > m_inside;
    MT::CTAtomic< Int32 > m_maxInside;
    MT::CTAtomic< UInt32 > m_reads;
};

/*-------------------------------------------------------------------------*/

/** An input that reports its reads to a counter, once it is armed (opening a stream reads the header, not counted) */
class COverlapInput : public CORE::CDynamicBufferAccess
{
    public:

    COverlapInput( CORE::CDynamicBuffer& buffer, COverlapCounter& counter )
        : CORE::CDynamicBufferAccess( buffer ), m_counter( counter ), m_isArmed( false ), m_isFirst( true ) {}

    void Arm( void ) { m_isArmed = true; }

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_isArmed )
            return CORE::CDynamicBufferAccess::Read( dest, esize, elements );

        m_counter.Enter( m_isFirst );
        m_isFirst = false;
        const UInt32 result = CORE::CDynamicBufferAccess::Read( dest, esize, elements );
        m_counter.Leave();
        return result;
    }

    private:

    COverlapCounter& m_counter;
    bool m_isArmed;
    bool m_isFirst;
};

/*-------------------------------------------------------------------------*/

/** Reads a source to its end on a thread of its own, or until told to stop */
class CDecodeThread : public MT::CActiveObject
{
    public:

    CDecodeThread( const AUDIO::TAudioSourcePtr& source, UInt32 chunkFrames, UInt32 frameBytes )
        : MT::CActiveObject(), m_source( source ), m_chunkFrames( chunkFrames ), m_frameBytes( frameBytes ), m_data(), done( 0 ), framesRead( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDynamicBuffer chunk( m_chunkFrames * m_frameBytes, true );
        while ( !m_source->IsEndOfStream() )
        {
            const UInt32 frames = m_source->ReadFrames( chunk.GetBufferPtr(), m_chunkFrames );
            if ( 0 == frames )
                break;
            m_data.Append( chunk.GetConstBufferPtr(), frames * m_frameBytes );
            framesRead.FetchAdd( frames );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitForDone( UInt32 timeoutInMs )
    {
        for ( UInt32 i=0; i<timeoutInMs && 0 == done.Load(); ++i )
            MT::ThreadDelay( 1 );
        return 0 != done.Load();
    }

    AUDIO::TAudioSourcePtr m_source;
    UInt32 m_chunkFrames;
    UInt32 m_frameBytes;
    CORE::CDynamicBuffer m_data;
    MT::CTAtomic< UInt32 > done;
    MT::CTAtomic< UInt32 > framesRead;
};

/*-------------------------------------------------------------------------*/

/** A mono 16 bit big endian .au file of a ramp, with an annotation */
void
MakeRampFile( CORE::CDynamicBuffer& file, UInt32 frames, const char* annotation )
{
    CORE::CDynamicBuffer samples;
    for ( UInt32 i=0; i<frames; ++i )
    {
        const Int16 value = (Int16) ( ( i * 7 ) % 20000 - 10000 );
        const UInt8 bytes[ 2 ] = { (UInt8) ( (UInt16) value >> 8 ), (UInt8) ( (UInt16) value & 0xFF ) };
        samples.Append( bytes, 2 );
    }
    MakeAuFile( file, 3, 8000, 1, samples.GetConstBufferPtr(), samples.GetDataSize(), annotation );
}

/*-------------------------------------------------------------------------*/

/**
 *  Decodes the same file through two sources of the codec at once, one thread each, and reports how many reads
 *  of the plugin were seen inside it at the same time. False when the decoded audio is not what a single decode gives.
 */
bool
DecodeTwoAtOnce( AUDIO::CAudioCodecRegistry::TAudioCodecPtr codec, Int32& maxOverlap )
{
    CORE::CDynamicBuffer file;
    MakeRampFile( file, 6000, GUCEF_NULL );

    COverlapCounter counter;
    COverlapInput* inputs[ 2 ];
    AUDIO::TAudioSourcePtr sources[ 2 ];
    CDecodeThread* threads[ 2 ];
    for ( UInt32 i=0; i<2; ++i )
    {
        inputs[ i ] = GUCEF_NEW COverlapInput( file, counter );
        CORE::CIOAccess::IOAccessPtr owner( inputs[ i ] );
        if ( !codec->OpenSource( owner, sources[ i ] ) )
            return false;
        inputs[ i ]->Arm();
        threads[ i ] = GUCEF_NEW CDecodeThread( sources[ i ], 500, 2 );
    }

    bool isOk = true;
    for ( UInt32 i=0; i<2; ++i )
        isOk = threads[ i ]->Activate() && isOk;
    for ( UInt32 i=0; i<2; ++i )
    {
        isOk = threads[ i ]->WaitForDone( 20000 ) && isOk;
        threads[ i ]->Deactivate( false, true );
    }

    for ( UInt32 i=0; i<2; ++i )
    {
        isOk = 6000 == threads[ i ]->framesRead.Load() && 12000 == threads[ i ]->m_data.GetDataSize() && isOk;
        // the samples are the ramp, 16 bit native endian
        const Int16* samples = (const Int16*) threads[ i ]->m_data.GetConstBufferPtr();
        for ( UInt32 f=0; f<6000 && isOk; ++f )
            isOk = samples[ f ] == (Int16) ( ( f * 7 ) % 20000 - 10000 );
        GUCEF_DELETE threads[ i ];
    }
    maxOverlap = counter.GetMaxInside();
    return isOk;
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
        params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );     // no delay: the test looks at the output frame by frame
        params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 22050, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        params.GetDeviceParams().SetBufferSizeInFrames( 256 );
        params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        // a plugin source takes the plugin's call lock: never read on the audio thread, so it plays through a stream
        // that is filled here, on the owner side (a RealtimeSanitizer build fails the render otherwise)
        AUDIO::CAudioStream* stream = GUCEF_NEW AUDIO::CAudioStream( source, 1000, 1, false );
        stream->Fill();
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( AUDIO::TAudioSourcePtr( stream ) );
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

    // ---- the optional additions of the plugin interface: threading model, stream tags and probe ----

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: A plugin without the optional entry points keeps working: one call at a time, no tags, no probe" )
    try
    {
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr au = FindCodec( "au" );
        AUDIO::CAudioCodecPluginManager::TAudioCodecCPluginPtr plugin = AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "au" );
        ASSERT_FALSE( au.IsNULL() || plugin.IsNULL() );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK == plugin->GetThreadingModel() );
        const AUDIO::TGucefAudioCodecPluginApi* api = plugin->GetLinkedApi();
        ASSERT_TRUE( GUCEF_NULL != api );
        ASSERT_TRUE( GUCEF_NULL == api->Threading_Model && GUCEF_NULL == api->Src_Meta_Data && GUCEF_NULL == api->Probe );

        CORE::CDynamicBuffer file;
        MakeRampFile( file, 1000, "LOOPSTART=10" );

        // no tags of the open stream, but the stream itself works
        CORE::CDynamicBufferAccess input( file );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( au->OpenSource( input, source ) );
        CORE::CValueList tags;
        ASSERT_FALSE( source->GetMetaData( tags ) );
        ASSERT_TRUE( 0 == tags.GetKeyCount() );
        Int16 frames[ 100 ];
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );
        ASSERT_TRUE( frames[ 1 ] == (Int16) 7 - 10000 );

        // a source that is not from a plugin has none either
        AUDIO::CAudioBuffer clipAudio( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 10 );
        AUDIO::CAudioClip clip( clipAudio );
        AUDIO::TAudioSourcePtr clipSource = clip.CreateSource();
        ASSERT_FALSE( clipSource->GetMetaData( tags ) );

        // no probe: the codec cannot tell, says so, and leaves the input where it was
        CORE::CDynamicBufferAccess probeInput( file );
        probeInput.Setpos( 5 );
        UInt32 confidence = 77;
        ASSERT_FALSE( au->Probe( probeInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );
        ASSERT_TRUE( 5 == probeInput.Tell() );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr found;
        probeInput.Setpos( 0 );
        ASSERT_FALSE( AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForData( probeInput, found ) );
        ASSERT_TRUE( 0 == probeInput.Tell() );

        // and the built in codecs have no probe either
        ASSERT_FALSE( FindCodec( "wav" )->Probe( probeInput, confidence ) );

        // streams of a plugin like this are decoded one at a time
        Int32 maxOverlap = 0;
        ASSERT_TRUE( DecodeTwoAtOnce( au, maxOverlap ) );
        ASSERT_TRUE( 1 == maxOverlap );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: A plugin with the optional entry points: stream tags are read from the open stream, the probe is used" )
    try
    {
        ASSERT_TRUE( LoadExtensionPlugin() );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr ext = FindCodec( "autest" );
        ASSERT_FALSE( ext.IsNULL() );
        ASSERT_FALSE( FindCodec( "sndtest" ).IsNULL() );
        AUDIO::CAudioCodecPluginManager::TAudioCodecCPluginPtr plugin = AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "autest" );
        ASSERT_FALSE( plugin.IsNULL() );
        const AUDIO::TGucefAudioCodecPluginApi* api = plugin->GetLinkedApi();
        ASSERT_TRUE( GUCEF_NULL != api->Threading_Model && GUCEF_NULL != api->Src_Meta_Data && GUCEF_NULL != api->Probe );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == plugin->GetThreadingModel() );

        // the plain plugin is still there and still declares nothing
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK == AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "au" )->GetThreadingModel() );

        CORE::CDynamicBuffer file;
        MakeRampFile( file, 1000, "LOOPSTART=250\nLoopLength=500\nTITLE=Theme" );

        // tags of the open stream, without opening the input again
        CORE::CDynamicBufferAccess input( file );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( ext->OpenSource( input, source ) );
        ASSERT_TRUE( 1000 == source->GetLengthInFrames() );
        Int16 frames[ 200 ];
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );
        CORE::CValueList tags;
        ASSERT_TRUE( source->GetMetaData( tags ) );
        ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "250" );
        ASSERT_TRUE( tags.GetValueAlways( "looplength" ).AsString() == "500" );
        ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Theme" );
        ASSERT_TRUE( tags.GetValueAlways( "encoding" ).AsString() == "linear" );
        ASSERT_TRUE( tags.GetValueAlways( "position" ).AsString() == "100" );        // it is about this stream, as it is now
        ASSERT_TRUE( tags.GetValueAlways( "comment" ).AsString().HasSubstr( "LOOPSTART=250" ) >= 0 );

        // asking did not disturb the stream
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );
        ASSERT_TRUE( 200 == source->GetPositionInFrames() );
        ASSERT_TRUE( frames[ 0 ] == (Int16) ( ( 100 * 7 ) % 20000 - 10000 ) );
        ASSERT_TRUE( source->SeekToFrame( 0 ) );
        ASSERT_TRUE( 100 == source->ReadFrames( frames, 100 ) );
        ASSERT_TRUE( frames[ 99 ] == (Int16) ( ( 99 * 7 ) % 20000 - 10000 ) );
        tags.Clear();
        ASSERT_TRUE( source->GetMetaData( tags ) );
        ASSERT_TRUE( tags.GetValueAlways( "position" ).AsString() == "100" );

        // the whole-input metadata of the codec is unchanged by the additions
        CORE::CValueList whole;
        CORE::CDynamicBufferAccess wholeInput( file );
        ASSERT_TRUE( ext->DecodeMetaData( wholeInput, whole ) );
        ASSERT_TRUE( whole.GetValueAlways( "frames" ).AsString() == "1000" );

        // the probe: a confident yes for .au data, a clear no for anything else, the input where it was
        CORE::CDynamicBufferAccess probeInput( file );
        UInt32 confidence = 0;
        ASSERT_TRUE( ext->Probe( probeInput, confidence ) );
        ASSERT_TRUE( 90 == confidence );
        ASSERT_TRUE( 0 == probeInput.Tell() );
        probeInput.Setpos( 3 );
        ASSERT_TRUE( ext->Probe( probeInput, confidence ) );
        ASSERT_TRUE( 0 == confidence );                                              // from offset 3 it is not the start of a file
        ASSERT_TRUE( 3 == probeInput.Tell() );

        // content lookup in the registry: finds the codec of a file that has no usable name
        AUDIO::CAudioCodecRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry();
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr found;
        ASSERT_FALSE( registry.TryFindCodecForFilename( "music/theme.dat", found ) );
        probeInput.Setpos( 0 );
        confidence = 0;
        ASSERT_TRUE( registry.TryFindCodecForData( probeInput, found, &confidence ) );
        ASSERT_TRUE( 90 == confidence );
        ASSERT_TRUE( found->GetType() == "autest" || found->GetType() == "sndtest" );
        ASSERT_TRUE( 0 == probeInput.Tell() );
        AUDIO::CAudioBuffer decoded;
        ASSERT_TRUE( found->DecodeAll( probeInput, decoded ) );
        ASSERT_TRUE( 1000 == decoded.GetFrameCount() );

        // nothing that any codec recognises
        const char notAu[ 32 ] = "RIFF....WAVEfmt ";
        CORE::CDynamicBuffer junk( notAu, sizeof( notAu ), true );
        CORE::CDynamicBufferAccess junkInput( junk );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr none;
        ASSERT_FALSE( registry.TryFindCodecForData( junkInput, none ) );
        ASSERT_TRUE( none.IsNULL() );
        ASSERT_TRUE( 0 == junkInput.Tell() );

        // once the plugin is gone the stream is closed and has no tags, and nothing probes any more
        source = AUDIO::TAudioSourcePtr();
        AUDIO::TAudioSourcePtr late;
        CORE::CDynamicBufferAccess lateInput( file );
        ASSERT_TRUE( ext->OpenSource( lateInput, late ) );
        ext = AUDIO::CAudioCodecRegistry::TAudioCodecPtr();
        found = AUDIO::CAudioCodecRegistry::TAudioCodecPtr();
        plugin = AUDIO::CAudioCodecPluginManager::TAudioCodecCPluginPtr();
        ASSERT_TRUE( UnloadExtensionPlugin() );
        tags.Clear();
        ASSERT_FALSE( late->GetMetaData( tags ) );
        ASSERT_TRUE( late->IsEndOfStream() || 0 == late->ReadFrames( frames, 10 ) );
        ASSERT_FALSE( registry.TryFindCodecForData( probeInput, none ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: Streams of a plugin that declares per source threading are decoded at the same time" )
    try
    {
        ASSERT_TRUE( LoadExtensionPlugin() );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr ext = FindCodec( "autest" );
        ASSERT_FALSE( ext.IsNULL() );
        ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "autest" )->GetThreadingModel() );

        // two threads, one stream each: both are inside the plugin at the same time, and both decode correctly
        Int32 maxOverlap = 0;
        ASSERT_TRUE( DecodeTwoAtOnce( ext, maxOverlap ) );
        ASSERT_TRUE( 2 == maxOverlap );

        // the same file through the plugin that does not declare it: never two at once
        ASSERT_TRUE( DecodeTwoAtOnce( FindCodec( "au" ), maxOverlap ) );
        ASSERT_TRUE( 1 == maxOverlap );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: The tags of a plugin source are readable through a CAudioStream that wraps it, and a source without tags says so" )
    try
    {
        ASSERT_TRUE( LoadExtensionPlugin() );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr ext = FindCodec( "autest" );
        ASSERT_FALSE( ext.IsNULL() );

        CORE::CDynamicBuffer file;
        MakeRampFile( file, 5000, "LOOPSTART=250\nTITLE=Theme" );

        // by hand and with the read ahead task running: the tags come through either way
        for ( UInt32 mode=0; mode<2; ++mode )
        {
            CORE::CDynamicBufferAccess input( file );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( ext->OpenSource( input, source ) );
            AUDIO::CAudioStream stream( source, 100, 1, 1 == mode );
            for ( UInt32 n=0; n<20; ++n )
            {
                CORE::CValueList tags;
                ASSERT_TRUE( stream.GetMetaData( tags ) );
                ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "250" );
                ASSERT_TRUE( tags.GetValueAlways( "title" ).AsString() == "Theme" );
            }
        }

        // a stream around a source without tags has none either
        AUDIO::CAudioBuffer clipAudio( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 10 );
        AUDIO::CAudioClip clip( clipAudio );
        CORE::CValueList none;
        ASSERT_FALSE( clip.CreateSource()->GetMetaData( none ) );
        AUDIO::CAudioStream clipStream( clip.CreateSource(), 100, 1, false );
        ASSERT_FALSE( clipStream.GetMetaData( none ) );
        ASSERT_TRUE( 0 == none.GetKeyCount() );

        ASSERT_TRUE( UnloadExtensionPlugin() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: A plugin that declares one call at a time, or something undefined, stays serialized and still decodes correctly" )
    try
    {
        // "lock" declares GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK, "unknown" a value that does not exist (taken as the same)
        const char* declared[] = { "lock", "unknown" };
        for ( UInt32 d=0; d<2; ++d )
        {
            ASSERT_TRUE( UnloadExtensionPlugin() );
            ASSERT_TRUE( LoadExtensionPlugin( declared[ d ] ) );
            AUDIO::CAudioCodecRegistry::TAudioCodecPtr ext = FindCodec( "autest" );
            ASSERT_FALSE( ext.IsNULL() );
            AUDIO::CAudioCodecPluginManager::TAudioCodecCPluginPtr plugin = AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( "autest" );
            ASSERT_TRUE( GUCEF_NULL != plugin->GetLinkedApi()->Threading_Model );
            ASSERT_TRUE( GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK == plugin->GetThreadingModel() );

            Int32 maxOverlap = 0;
            ASSERT_TRUE( DecodeTwoAtOnce( ext, maxOverlap ) );
            ASSERT_TRUE( 1 == maxOverlap );

            // the tags work in this mode too
            CORE::CDynamicBuffer file;
            MakeRampFile( file, 100, "LOOPSTART=5" );
            CORE::CDynamicBufferAccess input( file );
            AUDIO::TAudioSourcePtr source;
            ASSERT_TRUE( ext->OpenSource( input, source ) );
            CORE::CValueList tags;
            ASSERT_TRUE( source->GetMetaData( tags ) );
            ASSERT_TRUE( tags.GetValueAlways( "loopstart" ).AsString() == "5" );
            source = AUDIO::TAudioSourcePtr();
        }

        ASSERT_TRUE( UnloadExtensionPlugin() );
        SetTestPluginThreading( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CodecPlugin: Unloading a per source threading plugin while a thread decodes ends the stream without a crash" )
    try
    {
        ASSERT_TRUE( LoadExtensionPlugin() );
        AUDIO::CAudioCodecRegistry::TAudioCodecPtr ext = FindCodec( "autest" );
        ASSERT_FALSE( ext.IsNULL() );

        // long enough that the thread is still reading when the plugin goes: the input makes every read take a moment
        CORE::CDynamicBuffer file;
        MakeRampFile( file, 400000, GUCEF_NULL );
        COverlapCounter counter;
        COverlapInput* rawInput = GUCEF_NEW COverlapInput( file, counter );
        CORE::CIOAccess::IOAccessPtr owner( rawInput );
        AUDIO::TAudioSourcePtr source;
        ASSERT_TRUE( ext->OpenSource( owner, source ) );
        rawInput->Arm();

        CDecodeThread* thread = GUCEF_NEW CDecodeThread( source, 200, 2 );
        ASSERT_TRUE( thread->Activate() );
        for ( UInt32 i=0; i<2000 && thread->framesRead.Load() < 1000; ++i )
            MT::ThreadDelay( 1 );
        ASSERT_TRUE( thread->framesRead.Load() >= 1000 );

        ext = AUDIO::CAudioCodecRegistry::TAudioCodecPtr();
        ASSERT_TRUE( UnloadExtensionPlugin() );
        ASSERT_TRUE( thread->WaitForDone( 20000 ) );
        thread->Deactivate( false, true );
        ASSERT_TRUE( thread->framesRead.Load() < 400000 );             // it was cut off, not decoded to the end
        ASSERT_TRUE( source->IsEndOfStream() );
        GUCEF_DELETE thread;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

