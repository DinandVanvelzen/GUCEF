/*
 *  gucefVIDEO_TestApp: GUCEF test application for the VIDEO module
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

#include <string.h>
#include <math.h>

#ifndef GUCEFVIDEO_H
#include "gucefVIDEO.h"
#define GUCEFVIDEO_H
#endif /* GUCEFVIDEO_H ? */

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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H
#include "gucefIMAGE_CImageUpscalerRegistry.h"
#define GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALER_H
#include "gucefIMAGE_CImageUpscaler.h"
#define GUCEF_IMAGE_CIMAGEUPSCALER_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVideoWebm.h"

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

/** Its own group: the Smacker tests unload theirs */
const char* PluginGroup = "videocodecpluginwebm";

const UInt32 Width = 96;
const UInt32 Height = 64;
const UInt32 FrameCount = 30;
const UInt32 FramesPerSecond = 25;

/*-------------------------------------------------------------------------*/

bool
LoadWebmPlugin( void )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/videocodecpluginWEBM_d";
    const CORE::CString second = "$MODULEDIR$/videocodecpluginWEBM";
    #else
    const CORE::CString first = "$MODULEDIR$/videocodecpluginWEBM";
    const CORE::CString second = "$MODULEDIR$/videocodecpluginWEBM_d";
    #endif

    VIDEO::TVideoCodecPtr codec;
    if ( VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodec( "webm", codec ) )
        return true;
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, PluginGroup, true ) || pluginControl.AddPluginFromDir( second, PluginGroup, true );
}

/*-------------------------------------------------------------------------*/

VIDEO::TVideoCodecPtr
FindWebm( void )
{
    VIDEO::TVideoCodecPtr codec;
    VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodec( "webm", codec );
    return codec;
}

/*-------------------------------------------------------------------------*/

/** A picture with smooth gradients and a square that moves with the frame number */
void
MakePicture( UInt32 width, UInt32 height, UInt32 n, VIDEO::CVideoFrame& frame )
{
    frame.Prepare( VIDEO::CVideoFormat( width, height, IMAGE::PSF_RGBA, FramesPerSecond, 1 ) );
    UInt8* pixels = static_cast< UInt8* >( frame.GetPicture().GetDataPtr() );
    const UInt32 squareX = ( n * 3 ) % ( width - 16 ), squareY = height / 4;
    for ( UInt32 y=0; y<height; ++y )
    {
        UInt8* row = pixels + (size_t) y * frame.GetPitch();
        for ( UInt32 x=0; x<width; ++x )
        {
            const bool inSquare = x >= squareX && x < squareX + 16 && y >= squareY && y < squareY + 16;
            row[ x * 4 + 0 ] = inSquare ? 230 : (UInt8) ( 40 + x * 150 / width );
            row[ x * 4 + 1 ] = inSquare ? 200 : (UInt8) ( 60 + y * 120 / height );
            row[ x * 4 + 2 ] = inSquare ? 30 : (UInt8) ( 120 + ( x + y ) * 60 / ( width + height ) );
            row[ x * 4 + 3 ] = 255;
        }
    }
    frame.SetFrameIndex( n );
}

/*-------------------------------------------------------------------------*/

/** The peak signal to noise ratio of the colours of two RGBA pictures of the same size, in dB */
Float64
Psnr( const VIDEO::CVideoFrame& a, const VIDEO::CVideoFrame& b )
{
    GUCEF::vector< UInt8 > pa( (size_t) a.GetWidth() * a.GetHeight() * 4 ), pb( pa.size() );
    if ( a.GetWidth() != b.GetWidth() || a.GetHeight() != b.GetHeight() ||
         !a.ConvertTo32Bit( IMAGE::PSF_RGBA, &pa[ 0 ], a.GetWidth() * 4 ) || !b.ConvertTo32Bit( IMAGE::PSF_RGBA, &pb[ 0 ], b.GetWidth() * 4 ) )
        return 0.0;
    Float64 squares = 0.0;
    UInt64 count = 0;
    for ( size_t i=0; i<pa.size(); ++i )
    {
        if ( 3 == i % 4 )
            continue;
        const Float64 d = (Float64) pa[ i ] - (Float64) pb[ i ];
        squares += d * d;
        ++count;
    }
    if ( 0.0 == squares )
        return 100.0;
    return 10.0 * log10( 255.0 * 255.0 / ( squares / (Float64) count ) );
}

/*-------------------------------------------------------------------------*/

/** Collects the sound of each track as floats, as the WebM source gives it */
class CFloatCollector : public VIDEO::IVideoAudioSink
{
    public:

    explicit CFloatCollector( const VIDEO::TVideoSourcePtr& source )
        : m_channels()
        , samples()
    {
        for ( UInt32 t=0; t<source->GetAudioTrackCount(); ++t )
        {
            AUDIO::CAudioFormat format;
            source->GetAudioTrackFormat( t, format );
            m_channels.push_back( format.GetChannelCount() );
            samples.push_back( GUCEF::vector< Float32 >() );
        }
    }

    virtual void OnVideoAudio( UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( track >= samples.size() )
            return;
        const Float32* data = static_cast< const Float32* >( frames );
        samples[ track ].insert( samples[ track ].end(), data, data + (size_t) frameCount * m_channels[ track ] );
    }

    UInt32 GetFrames( UInt32 track ) const
    {
        return track < samples.size() ? (UInt32) ( samples[ track ].size() / m_channels[ track ] ) : 0;
    }

    /** The frequency of a sine in a channel, from the crossings of zero in the middle half of the sound */
    Float64 GetFrequency( UInt32 track, UInt32 channel, UInt32 sampleRate ) const
    {
        const UInt32 frames = GetFrames( track );
        const UInt32 first = frames / 4, last = frames * 3 / 4;
        UInt32 crossings = 0;
        for ( UInt32 f=first+1; f<last; ++f )
        {
            const Float32 before = samples[ track ][ (size_t) ( f - 1 ) * m_channels[ track ] + channel ];
            const Float32 now = samples[ track ][ (size_t) f * m_channels[ track ] + channel ];
            if ( ( before < 0.0f ) != ( now < 0.0f ) )
                ++crossings;
        }
        return last > first ? (Float64) crossings * 0.5 * sampleRate / (Float64) ( last - first ) : 0.0;
    }

    GUCEF::vector< UInt32 > m_channels;
    GUCEF::vector< GUCEF::vector< Float32 > > samples;
};

/*-------------------------------------------------------------------------*/

/** Writes the test movie into 'file': the pictures of MakePicture() and sines of 440 Hz (mono 16 bit 22050 Hz) and 1000 Hz (stereo float 44100 Hz) */
bool
WriteTestMovie( CORE::CDynamicBuffer& file, UInt32 audioTracks, UInt32 width, UInt32 height )
{
    VIDEO::TVideoCodecPtr codec = FindWebm();
    if ( codec.IsNULL() )
        return false;

    VIDEO::CIVideoCodec::TAudioFormatVector formats;
    if ( audioTracks > 0 )
        formats.push_back( AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ) );
    if ( audioTracks > 1 )
        formats.push_back( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );

    CORE::CValueList options;
    options.Set( "quality", "90" );
    options.Set( "speed", "6" );
    CORE::CDynamicBufferAccess output( file );
    VIDEO::TVideoSinkPtr sink;
    if ( !codec->OpenSink( output, VIDEO::CVideoFormat( width, height, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), formats, sink, options ) )
        return false;

    VIDEO::CVideoFrame frame;
    UInt64 written440 = 0, written1000 = 0;
    for ( UInt32 n=0; n<FrameCount; ++n )
    {
        MakePicture( width, height, n, frame );
        if ( !sink->WriteFrame( frame ) )
            return false;

        // the sound of this frame's time, as a movie interleaves it
        if ( audioTracks > 0 )
        {
            const UInt64 until = (UInt64) ( n + 1 ) * 22050 / FramesPerSecond;
            GUCEF::vector< Int16 > mono;
            for ( ; written440 < until; ++written440 )
                mono.push_back( (Int16) ( 12000.0 * sin( 2.0 * 3.14159265358979 * 440.0 * (Float64) written440 / 22050.0 ) ) );
            if ( !mono.empty() && !sink->WriteAudio( 0, &mono[ 0 ], (UInt32) mono.size() ) )
                return false;
        }
        if ( audioTracks > 1 )
        {
            const UInt64 until = (UInt64) ( n + 1 ) * 44100 / FramesPerSecond;
            GUCEF::vector< Float32 > stereo;
            for ( ; written1000 < until; ++written1000 )
            {
                const Float32 value = (Float32) ( 0.4 * sin( 2.0 * 3.14159265358979 * 1000.0 * (Float64) written1000 / 44100.0 ) );
                stereo.push_back( value );
                stereo.push_back( -value );
            }
            if ( !stereo.empty() && !sink->WriteAudio( 1, &stereo[ 0 ], (UInt32) ( stereo.size() / 2 ) ) )
                return false;
        }
    }
    return FrameCount == sink->GetFramesWritten() && sink->Finish() && !sink->HasFailed();
}

/*-------------------------------------------------------------------------*/

VIDEO::TVideoSourcePtr
OpenBuffer( const CORE::CDynamicBuffer& file )
{
    CORE::CDynamicBuffer* copy = GUCEF_NEW CORE::CDynamicBuffer( file );
    CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( copy, true ) );
    VIDEO::TVideoSourcePtr source;
    VIDEO::TVideoCodecPtr codec = FindWebm();
    if ( codec.IsNULL() || !codec->OpenSource( access, source ) )
        return VIDEO::TVideoSourcePtr();
    return source;
}

/*-------------------------------------------------------------------------*/

/** Counts the progress reports of a transcode, and can stop it after a number of frames */
class CProgressCounter : public VIDEO::CIVideoTranscodeProgress
{
    public:

    explicit CProgressCounter( UInt64 stopAfter ) : reports( 0 ), lastTotal( 0 ), m_stopAfter( stopAfter ) {}

    virtual bool OnTranscodeProgress( UInt64 framesDone, UInt64 framesTotal ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++reports;
        lastTotal = framesTotal;
        return framesDone < m_stopAfter;
    }

    UInt64 reports;
    UInt64 lastTotal;

    private:

    UInt64 m_stopAfter;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVideoWebmTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING WebM video codec plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefVIDEO.WebM" );

    const bool loaded = LoadWebmPlugin();

    GUCEF_TESTFW_TESTCASE( "WebM: The plugin loads through the C interface, registers webm and mkv and can encode" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !loaded )
        GUCEF_TESTFW_SKIP( "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( loaded );
        VIDEO::TVideoCodecPtr codec = FindWebm();
        ASSERT_TRUE( !codec.IsNULL() && codec->CanEncode() );
        VIDEO::TVideoCodecPtr byName;
        ASSERT_TRUE( VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodecForFilename( "movies/opening.webm", byName ) );
        ASSERT_TRUE( VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodecForFilename( "movies/opening.mkv", byName ) );

        // the Smacker codec only decodes
        VIDEO::TVideoCodecPtr smk;
        if ( VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodec( "smk", smk ) )
        {
            ASSERT_FALSE( smk->CanEncode() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !loaded )
        return;

    CORE::CDynamicBuffer movie;
    const bool written = WriteTestMovie( movie, 1, Width, Height );

    GUCEF_TESTFW_TESTCASE( "WebM: A movie written with the sink reads back with its size, rate, frames, pictures and sound" )
    try
    {
        ASSERT_TRUE( written && movie.GetDataSize() > 1000 );
        ASSERT_TRUE( 0 == memcmp( movie.GetConstBufferPtr(), "\x1A\x45\xDF\xA3", 4 ) );       // an EBML file

        VIDEO::TVideoSourcePtr source = OpenBuffer( movie );
        ASSERT_FALSE( source.IsNULL() );
        if ( !source.IsNULL() )
        {
            const VIDEO::CVideoFormat& format = source->GetFormat();
            ASSERT_TRUE( Width == format.GetWidth() && Height == format.GetHeight() && IMAGE::PSF_RGBA == format.GetPixelFormat() );
            ASSERT_TRUE( FramesPerSecond == format.GetFrameRateNumerator() && 1 == format.GetFrameRateDenominator() );
            ASSERT_TRUE( FrameCount == format.GetFrameCount() );
            ASSERT_TRUE( 1 == source->GetAudioTrackCount() );
            AUDIO::CAudioFormat audioFormat;
            ASSERT_TRUE( source->GetAudioTrackFormat( 0, audioFormat ) && 48000 == audioFormat.GetSampleRate() &&
                         1 == audioFormat.GetChannelCount() && AUDIO::SAMPLETYPE_FLOAT32 == audioFormat.GetSampleType() );

            CFloatCollector sound( source );
            VIDEO::CVideoFrame frame, expected;
            Float64 worst = 100.0;
            for ( UInt32 n=0; n<FrameCount; ++n )
            {
                ASSERT_TRUE( source->DecodeNextFrame( frame, &sound ) );
                ASSERT_TRUE( n == frame.GetFrameIndex() && (UInt64) n * 40000 == frame.GetTimeInMicroSeconds() );
                MakePicture( Width, Height, n, expected );
                const Float64 psnr = Psnr( frame, expected );
                if ( psnr < worst )
                    worst = psnr;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebM: the worst picture after the round trip has a PSNR of " << worst << " dB" );
            ASSERT_TRUE( worst > 30.0 );     // the edges of the coloured square lose some colour to the 4:2:0 chroma
            ASSERT_FALSE( source->DecodeNextFrame( frame, &sound ) );
            ASSERT_TRUE( source->IsEndOfStream() && !source->HasFailed() );

            // 1.2 seconds of sound at 48 kHz, give or take the last Opus frame; the tone is still 440 Hz
            const UInt32 frames = sound.GetFrames( 0 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebM: " << frames << " frames of sound, " << sound.GetFrequency( 0, 0, 48000 ) << " Hz" );
            ASSERT_TRUE( frames >= 57600 - 960 && frames <= 57600 + 2 * 960 );
            ASSERT_TRUE( fabs( sound.GetFrequency( 0, 0, 48000 ) - 440.0 ) < 5.0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: Two audio tracks of other rates and channels come out as Opus at 48 kHz with their own sound" )
    try
    {
        CORE::CDynamicBuffer twoTracks;
        ASSERT_TRUE( WriteTestMovie( twoTracks, 2, Width, Height ) );
        VIDEO::TVideoSourcePtr source = OpenBuffer( twoTracks );
        ASSERT_TRUE( !source.IsNULL() && 2 == source->GetAudioTrackCount() );
        if ( !source.IsNULL() && 2 == source->GetAudioTrackCount() )
        {
            AUDIO::CAudioFormat stereo;
            ASSERT_TRUE( source->GetAudioTrackFormat( 1, stereo ) && 2 == stereo.GetChannelCount() && 48000 == stereo.GetSampleRate() );
            CFloatCollector sound( source );
            VIDEO::CVideoFrame frame;
            while ( source->DecodeNextFrame( frame, &sound ) ) {}
            ASSERT_TRUE( !source->HasFailed() && FrameCount == source->GetPositionInFrames() );
            ASSERT_TRUE( fabs( sound.GetFrequency( 0, 0, 48000 ) - 440.0 ) < 5.0 );
            ASSERT_TRUE( fabs( sound.GetFrequency( 1, 0, 48000 ) - 1000.0 ) < 10.0 && fabs( sound.GetFrequency( 1, 1, 48000 ) - 1000.0 ) < 10.0 );

            // the right channel was written as the opposite of the left one
            const UInt32 middle = sound.GetFrames( 1 ) / 2;
            const Float32 left = sound.samples[ 1 ][ (size_t) middle * 2 ], right = sound.samples[ 1 ][ (size_t) middle * 2 + 1 ];
            ASSERT_TRUE( fabs( left + right ) < 0.1f );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: Seeking gives the frame completely drawn, also from a frame after a key frame" )
    try
    {
        VIDEO::TVideoSourcePtr source = OpenBuffer( movie );
        ASSERT_TRUE( !source.IsNULL() && source->IsSeekable() );
        if ( !source.IsNULL() )
        {
            VIDEO::CVideoFrame frame, expected;
            const UInt32 targets[] = { 17, 3, 29, 0 };
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( source->SeekToFrame( targets[ i ] ) );
                ASSERT_TRUE( source->DecodeNextFrame( frame, GUCEF_NULL ) );
                MakePicture( Width, Height, targets[ i ], expected );
                ASSERT_TRUE( targets[ i ] == frame.GetFrameIndex() && Psnr( frame, expected ) > 30.0 );
            }
            ASSERT_FALSE( source->SeekToFrame( FrameCount + 1 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: Damaged and truncated files do not open or end with a failure, without crashing" )
    try
    {
        CORE::CDynamicBuffer garbage;
        for ( UInt32 i=0; i<4096; ++i )
        {
            const UInt8 value = (UInt8) ( i * 37 + 11 );
            garbage.Append( &value, 1 );
        }
        ASSERT_TRUE( OpenBuffer( garbage ).IsNULL() );

        CORE::CDynamicBuffer empty;
        ASSERT_TRUE( OpenBuffer( empty ).IsNULL() );

        // cut in the middle: it opens with what is there, or not at all, and decoding stops
        CORE::CDynamicBuffer half;
        half.Append( movie.GetConstBufferPtr(), movie.GetDataSize() / 2 );
        VIDEO::TVideoSourcePtr source = OpenBuffer( half );
        if ( !source.IsNULL() )
        {
            VIDEO::CVideoFrame frame;
            UInt32 decoded = 0;
            while ( decoded < FrameCount + 5 && source->DecodeNextFrame( frame, GUCEF_NULL ) )
                ++decoded;
            ASSERT_TRUE( decoded < FrameCount && source->IsEndOfStream() );
        }

        // a damaged picture inside: the source ends with a failure or shows what libvpx makes of it
        CORE::CDynamicBuffer damaged( movie );
        UInt8* bytes = static_cast< UInt8* >( damaged.GetBufferPtr() );
        for ( UInt32 i=damaged.GetDataSize() / 3; i<damaged.GetDataSize() / 3 + 200; ++i )
            bytes[ i ] ^= 0x5A;
        VIDEO::TVideoSourcePtr hurt = OpenBuffer( damaged );
        if ( !hurt.IsNULL() )
        {
            VIDEO::CVideoFrame frame;
            UInt32 decoded = 0;
            while ( decoded < FrameCount + 5 && hurt->DecodeNextFrame( frame, GUCEF_NULL ) )
                ++decoded;
            ASSERT_TRUE( decoded <= FrameCount && hurt->IsEndOfStream() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: The meta data tells the size, the frames, the duration and the sound" )
    try
    {
        CORE::CDynamicBufferAccess access( movie );
        CORE::CValueList metaData;
        VIDEO::TVideoCodecPtr codec = FindWebm();
        ASSERT_TRUE( !codec.IsNULL() && codec->DecodeMetaData( access, metaData ) );
        ASSERT_TRUE( "96" == metaData.GetValueAlways( "width" ).AsString() && "64" == metaData.GetValueAlways( "height" ).AsString() );
        ASSERT_TRUE( "30" == metaData.GetValueAlways( "frames" ).AsString() && "1200" == metaData.GetValueAlways( "durationInMs" ).AsString() );
        ASSERT_TRUE( "1" == metaData.GetValueAlways( "audioTracks" ).AsString() && "48000" == metaData.GetValueAlways( "audioTrack0.sampleRate" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: A sink refuses pictures of another size and a source without a picture track does not open" )
    try
    {
        VIDEO::TVideoCodecPtr codec = FindWebm();
        CORE::CDynamicBuffer file;
        CORE::CDynamicBufferAccess output( file );
        VIDEO::TVideoSinkPtr sink;
        ASSERT_TRUE( !codec.IsNULL() && codec->OpenSink( output, VIDEO::CVideoFormat( Width, Height, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), VIDEO::CIVideoCodec::TAudioFormatVector(), sink ) );
        if ( !sink.IsNULL() )
        {
            VIDEO::CVideoFrame other;
            MakePicture( Width / 2, Height, 0, other );
            ASSERT_FALSE( sink->WriteFrame( other ) );
            ASSERT_TRUE( sink->HasFailed() );
            ASSERT_FALSE( sink->Finish() );
        }

        // no frame rate: no sink
        VIDEO::TVideoSinkPtr noRate;
        ASSERT_FALSE( codec->OpenSink( output, VIDEO::CVideoFormat( Width, Height, IMAGE::PSF_RGBA, 0, 1 ), VIDEO::CIVideoCodec::TAudioFormatVector(), noRate ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: The transcoder takes a movie to a sink, upscaled with an image upscaler, and reports its progress" )
    try
    {
        // an upscale twice as large with mmpx, which gucefIMAGE always has
        VIDEO::TVideoSourcePtr source = OpenBuffer( movie );
        ASSERT_FALSE( source.IsNULL() );
        VIDEO::TVideoCodecPtr codec = FindWebm();
        CORE::CDynamicBuffer upscaledFile;
        {
            CORE::IOAccessPtr output( GUCEF_NEW CORE::CDynamicBufferAccess( &upscaledFile, false ) );
            VIDEO::CIVideoCodec::TAudioFormatVector formats;
            AUDIO::CAudioFormat audioFormat;
            source->GetAudioTrackFormat( 0, audioFormat );
            formats.push_back( audioFormat );
            VIDEO::TVideoSinkPtr sink;
            CORE::CValueList options;
            options.Set( "quality", "90" );
            options.Set( "speed", "6" );
            ASSERT_TRUE( codec->OpenSink( output, VIDEO::CVideoFormat( Width * 2, Height * 2, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), formats, sink, options ) );

            CProgressCounter progress( FrameCount + 1 );
            ASSERT_TRUE( VIDEO::CVideoTranscoder::Transcode( source, sink, "mmpx", 2, &progress, 4 ) );
            ASSERT_TRUE( FrameCount == progress.reports && FrameCount == progress.lastTotal );
        }
        VIDEO::TVideoSourcePtr upscaled = OpenBuffer( upscaledFile );
        ASSERT_TRUE( !upscaled.IsNULL() && Width * 2 == upscaled->GetFormat().GetWidth() && FrameCount == upscaled->GetFormat().GetFrameCount() );
        if ( !upscaled.IsNULL() )
        {
            // every frame in its place: the decoded picture is that frame upscaled, not one of its neighbours
            IMAGE::CImageUpscaler* mmpx = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( "mmpx" );
            CFloatCollector sound( upscaled );
            VIDEO::CVideoFrame frame, original, expected;
            UInt32 n = 0, misplaced = 0;
            while ( upscaled->DecodeNextFrame( frame, &sound ) )
            {
                MakePicture( Width, Height, n, original );
                IMAGE::TPixelMapPtr rgba, big;
                if ( GUCEF_NULL != mmpx && original.ConvertTo32Bit( IMAGE::PSF_RGBA, rgba ) && mmpx->Upscale( *rgba, 2, big, GUCEF_NULL ) && !big.IsNULL() )
                {
                    expected.Prepare( VIDEO::CVideoFormat( Width * 2, Height * 2, IMAGE::PSF_RGBA, FramesPerSecond, 1 ) );
                    memcpy( expected.GetPicture().GetDataPtr(), big->GetDataPtr(), (size_t) Width * 2 * Height * 2 * 4 );
                    if ( Psnr( frame, expected ) < 28.0 )
                        ++misplaced;
                }
                ++n;
            }
            ASSERT_TRUE( !upscaled->HasFailed() && FrameCount == upscaled->GetPositionInFrames() && 0 == misplaced );
            ASSERT_TRUE( fabs( sound.GetFrequency( 0, 0, 48000 ) - 440.0 ) < 5.0 );
        }

        // a sink that does not fit the factor is refused
        VIDEO::TVideoSourcePtr again = OpenBuffer( movie );
        CORE::CDynamicBuffer wrongFile;
        CORE::CDynamicBufferAccess wrongOutput( wrongFile );
        VIDEO::TVideoSinkPtr wrong;
        ASSERT_TRUE( codec->OpenSink( wrongOutput, VIDEO::CVideoFormat( Width * 3, Height * 3, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), VIDEO::CIVideoCodec::TAudioFormatVector(), wrong ) );
        ASSERT_FALSE( VIDEO::CVideoTranscoder::Transcode( again, wrong, "mmpx", 2, GUCEF_NULL ) );

        // the progress can stop it
        VIDEO::TVideoSourcePtr third = OpenBuffer( movie );
        CORE::CDynamicBuffer stoppedFile;
        CORE::CDynamicBufferAccess stoppedOutput( stoppedFile );
        VIDEO::TVideoSinkPtr stopped;
        ASSERT_TRUE( codec->OpenSink( stoppedOutput, VIDEO::CVideoFormat( Width, Height, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), VIDEO::CIVideoCodec::TAudioFormatVector(), stopped ) );
        CProgressCounter stopAt( 5 );
        ASSERT_FALSE( VIDEO::CVideoTranscoder::Transcode( third, stopped, CORE::CString::Empty, 1, &stopAt ) );
        ASSERT_TRUE( 5 == stopAt.reports );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: A transcode as a task finishes with its status, and stops when it is cancelled" )
    try
    {
        VIDEO::TVideoCodecPtr codec = FindWebm();
        CORE::CDynamicBuffer* outputFile = GUCEF_NEW CORE::CDynamicBuffer();
        CORE::IOAccessPtr output( GUCEF_NEW CORE::CDynamicBufferAccess( outputFile, true ) );
        VIDEO::TVideoSinkPtr sink;
        ASSERT_TRUE( codec->OpenSink( output, VIDEO::CVideoFormat( Width, Height, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), VIDEO::CIVideoCodec::TAudioFormatVector(), sink ) );
        VIDEO::TVideoTranscodeStatusPtr status;
        CORE::CFutureResult future = VIDEO::CVideoTranscoder::TranscodeAsync( OpenBuffer( movie ), sink, CORE::CString::Empty, 1, status, "gucefVIDEO_TestApp" );
        sink.Unlink();
        ASSERT_TRUE( future.HasAFuture() && !status.IsNULL() );
        future.Await( 60000 );
        ASSERT_TRUE( future.IsReady() && VIDEO::CVideoTranscoder::GetAsyncResult( future ) );
        ASSERT_TRUE( status->IsFinished() && status->IsSuccessful() && FrameCount == status->GetFramesDone() && FrameCount == status->GetFramesTotal() );
        output.Unlink();

        // cancelled at once: it ends without completing the output
        CORE::CDynamicBuffer cancelledFile;
        CORE::IOAccessPtr cancelledOutput( GUCEF_NEW CORE::CDynamicBufferAccess( &cancelledFile, false ) );
        VIDEO::TVideoSinkPtr cancelledSink;
        ASSERT_TRUE( codec->OpenSink( cancelledOutput, VIDEO::CVideoFormat( Width * 2, Height * 2, IMAGE::PSF_RGBA, FramesPerSecond, 1 ), VIDEO::CIVideoCodec::TAudioFormatVector(), cancelledSink ) );
        VIDEO::TVideoTranscodeStatusPtr cancelledStatus;
        CORE::CFutureResult cancelled = VIDEO::CVideoTranscoder::TranscodeAsync( OpenBuffer( movie ), cancelledSink, "mmpx", 2, cancelledStatus, "gucefVIDEO_TestApp" );
        cancelledSink.Unlink();
        cancelled.RequestCancellation();
        cancelled.Await( 60000 );
        ASSERT_TRUE( cancelled.IsReady() );
        // a task cancelled before it started never runs: the future tells it is over, the status stays unfinished
        ASSERT_FALSE( VIDEO::CVideoTranscoder::GetAsyncResult( cancelled ) );
        ASSERT_TRUE( !cancelledStatus->IsSuccessful() && cancelledStatus->GetFramesDone() < FrameCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebM: A movie plays through the player to its end" )
    try
    {
        VIDEO::CVideoPlayer player;
        ASSERT_TRUE( player.Open( OpenBuffer( movie ) ) && player.Start() );
        UInt32 pictures = 0;
        const UInt64 started = MT::PrecisionTickCount();
        while ( !player.IsEnded() && ( MT::PrecisionTickCount() - started ) < 10 * MT::PrecisionTimerResolution() )
        {
            if ( player.Update() )
                ++pictures;
            MT::PrecisionDelay( 5 );
        }
        ASSERT_TRUE( player.IsEnded() && VIDEO::VIDEOEND_FINISHED == player.GetEndReason() );
        ASSERT_TRUE( pictures > FrameCount / 2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
