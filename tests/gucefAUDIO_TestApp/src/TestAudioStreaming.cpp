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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioCodecSupport.h"
#include "TestAudioCodecMp3Support.h"
#include "TestAudioStreaming.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Checks 'call' for every format in 'set', and says which format failed */
#define CHECK_EACH_FORMAT( set, call ) \
    for ( UInt32 formatIndex=0; formatIndex<(set).count; ++formatIndex ) \
    { \
        SFormat& format = (set).items[ formatIndex ]; \
        const bool formatOk = call; \
        if ( !formatOk ) \
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: failed for the format " << format.name ); \
        ASSERT_TRUE( formatOk ); \
    }

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

typedef GUCEF::vector< Float32 > TSamples;

/*-------------------------------------------------------------------------*/

/** A file of one format with what a plain decode of it gives */
struct SFormat
{
    const char* name;
    TCodecPtr codec;
    UInt32 rate;
    bool exactSeek;                 /**< seeks give the same samples as reading on */
    bool settlesAfterSeek;          /**< the frames right after a seek are only close to reading on (the decoder has to settle) */
    CORE::CDynamicBuffer file;
    AUDIO::CAudioBuffer reference;

    SFormat( void ) : name( "" ), codec(), rate( 44100 ), exactSeek( true ), settlesAfterSeek( false ), file(), reference() {}
};

const UInt32 MaxFormats = 6;

struct SFormatSet
{
    SFormat items[ MaxFormats ];
    UInt32 count;

    SFormatSet( void ) : count( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/** A stereo file of noise: 'kind' is flac, vorbis, opus, mp3 or au */
bool
MakeFile( const char* kind, TCodecPtr codec, UInt32 frames, UInt32 seed, SFormat& format )
{
    format.name = kind;
    format.codec = codec;
    format.exactSeek = 0 == strcmp( kind, "flac" ) || 0 == strcmp( kind, "vorbis" ) || 0 == strcmp( kind, "au" );
    format.settlesAfterSeek = 0 == strcmp( kind, "opus" );
    if ( 0 == strcmp( kind, "mp3" ) )
    {
        format.rate = 44100;
        MakeStream( format.file, 2, frames / FrameSamples + 1, true, 576, 1500, seed );
    }
    else
    {
        format.rate = 0 == strcmp( kind, "opus" ) ? 48000 : 44100;
        CORE::CValueList options;
        if ( 0 == strcmp( kind, "vorbis" ) )
            options.Set( "quality", "0.5" );
        else
        if ( 0 == strcmp( kind, "opus" ) )
            options.Set( "bitrate", "128000" );
        const AUDIO::CAudioBuffer audio = MakeNoise( AUDIO::CAudioFormat( format.rate, 2, AUDIO::SAMPLETYPE_INT16 ), frames, 0.3f, seed );
        if ( !Encode( codec, audio, format.file, options ) )
            return false;
    }
    return DecodeFile( codec, format.file, format.reference ) && format.reference.GetFrameCount() > 1000;
}

/*-------------------------------------------------------------------------*/

/** One file per codec that is there. The codecs with the plugin lock (au) come only when 'withAu' */
void
BuildFormats( SFormatSet& set, UInt32 frames, UInt32 seed, bool withAu )
{
    set.count = 0;
    TCodecPtr flac = FindCodec( "flac" );
    if ( !flac.IsNULL() && MakeFile( "flac", flac, frames, seed, set.items[ set.count ] ) )
        ++set.count;
    TCodecPtr ogg = FindCodec( "ogg" );
    if ( !ogg.IsNULL() && MakeFile( "vorbis", ogg, frames, seed, set.items[ set.count ] ) )
        ++set.count;
    TCodecPtr opus = FindCodec( "opus" );
    if ( !opus.IsNULL() && MakeFile( "opus", opus, frames, seed, set.items[ set.count ] ) )
        ++set.count;
    TCodecPtr mp3 = FindCodec( "mp3" );
    if ( !mp3.IsNULL() && MakeFile( "mp3", mp3, frames, seed, set.items[ set.count ] ) )
        ++set.count;
    TCodecPtr au = FindCodec( "au" );
    if ( withAu && !au.IsNULL() && MakeFile( "au", au, frames, seed, set.items[ set.count ] ) )
        ++set.count;
}

/*-------------------------------------------------------------------------*/

/** Reads up to 'frames' from the source, waiting for a stream that has not read ahead far enough, until the end or 10 seconds without a frame */
UInt32
ReadSome( AUDIO::IAudioSource& source, void* destination, UInt32 frames, UInt32 bytesPerFrame )
{
    UInt32 total = 0;
    UInt32 idleMs = 0;
    while ( total < frames && idleMs < 10000 )
    {
        const UInt32 got = source.ReadFrames( static_cast< UInt8* >( destination ) + (size_t) total * bytesPerFrame, frames - total );
        total += got;
        if ( 0 == got )
        {
            if ( source.IsEndOfStream() )
                break;
            MT::ThreadDelay( 1 );
            ++idleMs;
        }
        else
            idleMs = 0;
    }
    return total;
}

/*-------------------------------------------------------------------------*/

/** Reads a stream to its end the way a render side does, in odd sized chunks, waiting for the read ahead. 'fillByHand' calls Fill() before each read */
bool
ReadStreamToEnd( AUDIO::CAudioStream& stream, AUDIO::CAudioBuffer& audio, bool fillByHand, UInt32 chunk = 733 )
{
    const AUDIO::CAudioFormat format = stream.GetFormat();
    const UInt32 frameBytes = format.GetBytesPerFrame();
    CORE::CDynamicBuffer all;
    CORE::CDynamicBuffer buffer( chunk * frameBytes, true );
    UInt64 total = 0;
    UInt32 idleRounds = 0;
    while ( !stream.IsEndOfStream() && total < 20000000 && idleRounds < 20000 )
    {
        if ( fillByHand )
            stream.Fill();
        const UInt32 got = stream.ReadFrames( buffer.GetBufferPtr(), chunk );
        if ( 0 == got )
        {
            ++idleRounds;
            MT::ThreadDelay( 1 );
            continue;
        }
        idleRounds = 0;
        all.Append( buffer.GetConstBufferPtr(), got * frameBytes );
        total += got;
    }
    audio = AUDIO::CAudioBuffer( format, all.GetConstBufferPtr(), (UInt32) total );
    return idleRounds < 20000;
}

/*-------------------------------------------------------------------------*/

/** A source that is slow: a pause before every read of at most 'maxFrames' frames. Counts its destruction */
class CSlowSource : public AUDIO::IAudioSource
{
    public:

    CSlowSource( const AUDIO::TAudioSourcePtr& inner, UInt32 delayInMs, UInt32 maxFrames, MT::CTAtomic< UInt32 >* destroyed )
        : m_inner( inner ), m_delayInMs( delayInMs ), m_maxFrames( maxFrames ), m_destroyed( destroyed ) {}

    virtual ~CSlowSource() GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != m_destroyed )
            m_destroyed->FetchAdd( 1 );
    }

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetFormat(); }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_delayInMs > 0 )
            MT::ThreadDelay( m_delayInMs );
        return m_inner->ReadFrames( destination, frameCapacity < m_maxFrames ? frameCapacity : m_maxFrames );
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->IsEndOfStream(); }
    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->IsSeekable(); }
    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE { return m_inner->SeekToFrame( frame ); }
    virtual bool GetMetaData( CORE::CValueList& metaData ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetMetaData( metaData ); }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetPositionInFrames(); }
    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetLengthInFrames(); }

    private:

    AUDIO::TAudioSourcePtr m_inner;
    UInt32 m_delayInMs;
    UInt32 m_maxFrames;
    MT::CTAtomic< UInt32 >* m_destroyed;
};

/*-------------------------------------------------------------------------*/

AUDIO::CAudioEngineParams
EngineParams( UInt32 rate, bool threaded )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( threaded ? AUDIO::AUDIOTHREADING_THREADED : AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( rate, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

/*-------------------------------------------------------------------------*/

AUDIO::CNullAudioDevice*
NullDeviceOf( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr device = engine.GetDevice();
    return device.IsNULL() ? GUCEF_NULL : device->AsNullDevice();
}

/*-------------------------------------------------------------------------*/

/** What the null device of the engine rendered, as float samples */
void
CopyHeard( AUDIO::CAudioEngine& engine, TSamples& heard )
{
    heard.clear();
    AUDIO::CNullAudioDevice* device = NullDeviceOf( engine );
    if ( GUCEF_NULL == device )
        return;
    const AUDIO::CAudioBuffer& rendered = device->GetRenderedAudio();
    const UInt32 channels = rendered.GetFormat().GetChannelCount();
    heard.reserve( (size_t) rendered.GetFrameCount() * channels );
    for ( UInt32 f=0; f<rendered.GetFrameCount(); ++f )
        for ( UInt32 c=0; c<channels; ++c )
            heard.push_back( rendered.GetSampleAsFloat( f, c ) );
}

/*-------------------------------------------------------------------------*/

void
AppendWithoutSilentFrames( TSamples& out, const TSamples& in, UInt32 channels )
{
    for ( size_t f=0; f+channels<=in.size(); f+=channels )
    {
        bool isSilent = true;
        for ( UInt32 c=0; c<channels && isSilent; ++c )
            isSilent = 0.0f == in[ f + c ];
        if ( !isSilent )
            for ( UInt32 c=0; c<channels; ++c )
                out.push_back( in[ f + c ] );
    }
}

/*-------------------------------------------------------------------------*/

TSamples
SamplesOf( const AUDIO::CAudioBuffer& audio )
{
    TSamples samples;
    const UInt32 channels = audio.GetFormat().GetChannelCount();
    samples.reserve( (size_t) audio.GetFrameCount() * channels );
    for ( UInt32 f=0; f<audio.GetFrameCount(); ++f )
        for ( UInt32 c=0; c<channels; ++c )
            samples.push_back( audio.GetSampleAsFloat( f, c ) );
    return samples;
}

/*-------------------------------------------------------------------------*/

/**
 *  What the engine rendered is the reference 'repeats' times in order, once the frames of silence are left out of
 *  both: the gaps of an underrun and of a restart by seek are silence and nothing else may be missing, repeated or out of order.
 *  With 'heardIsPrefix' the rendering only has to be the start of the repeats (a stream that loops forever).
 */
bool
SameWhenSilenceIsLeftOut( const TSamples& heard, const AUDIO::CAudioBuffer& reference, UInt32 repeats, bool heardIsPrefix )
{
    const UInt32 channels = reference.GetFormat().GetChannelCount();
    TSamples got;
    AppendWithoutSilentFrames( got, heard, channels );
    TSamples once;
    AppendWithoutSilentFrames( once, SamplesOf( reference ), channels );
    TSamples expected;
    for ( UInt32 r=0; r<repeats; ++r )
        expected.insert( expected.end(), once.begin(), once.end() );

    if ( heardIsPrefix )
    {
        if ( got.size() > expected.size() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: heard " << got.size() << " samples, more than the " << expected.size() << " expected" );
            return false;
        }
        expected.resize( got.size() );
    }
    else
    if ( got.size() != expected.size() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: heard " << got.size() << " samples, expected " << expected.size() );
        return false;
    }

    for ( size_t i=0; i<got.size(); ++i )
    {
        if ( fabsf( got[ i ] - expected[ i ] ) > 2e-6f )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: first difference at sample " << i << " of " << got.size() << ": " << got[ i ] << " instead of " << expected[ i ] );
            return false;
        }
    }
    return got.size() > 0;
}

/*-------------------------------------------------------------------------*/

/** Renders (a block per millisecond) until the voice has ended, and a few blocks more. False when it does not end in time */
bool
RenderUntilEnded( AUDIO::CAudioEngine& engine, const AUDIO::TAudioVoicePtr& voice, bool threaded, UInt32 maxBlocks = 30000 )
{
    AUDIO::CNullAudioDevice* device = NullDeviceOf( engine );
    if ( GUCEF_NULL == device )
        return false;
    for ( UInt32 i=0; i<maxBlocks; ++i )
    {
        device->Render( 256 );
        if ( !threaded )
            engine.Update();
        if ( AUDIO::AUDIOVOICESTATE_ENDED == voice->GetState() )
        {
            for ( UInt32 extra=0; extra<4; ++extra )
                device->Render( 256 );
            return true;
        }
        MT::ThreadDelay( 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Renders about 'blocks' blocks, a block per millisecond */
void
RenderBlocks( AUDIO::CAudioEngine& engine, UInt32 blocks, bool threaded )
{
    AUDIO::CNullAudioDevice* device = NullDeviceOf( engine );
    for ( UInt32 i=0; i<blocks && GUCEF_NULL != device; ++i )
    {
        device->Render( 256 );
        if ( !threaded )
            engine.Update();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

AUDIO::TAudioSourcePtr
OpenPossiblySlow( SFormat& format, UInt32 delayInMs, UInt32 maxFrames, MT::CTAtomic< UInt32 >* destroyed = GUCEF_NULL )
{
    AUDIO::TAudioSourcePtr source;
    if ( !OpenSource( format.codec, format.file, source ) )
        return AUDIO::TAudioSourcePtr();
    if ( 0 == delayInMs && GUCEF_NULL == destroyed )
        return source;
    return AUDIO::TAudioSourcePtr( GUCEF_NEW CSlowSource( source, delayInMs, maxFrames, destroyed ) );
}

/*-------------------------------------------------------------------------*/

struct SPlayed
{
    TSamples heard;
    UInt64 underruns;
    bool ended;
    SPlayed( void ) : heard(), underruns( 0 ), ended( false ) {}
};

/**
 *  Plays the file through a stream with the read ahead task and a voice of the real engine, with the engine thread. The
 *  voice plays 'voiceLoops' times, the stream loops 'streamLoops' times by itself. A 'sourceDelayInMs' above 0 makes the
 *  source slow.
 */
bool
PlayThroughEngine( SFormat& format, UInt32 streamLoops, UInt32 voiceLoops, UInt32 sourceDelayInMs, SPlayed& played )
{
    AUDIO::CAudioEngine engine;
    if ( !engine.Initialize( EngineParams( format.rate, true ) ) || engine.GetMixFormat().GetSampleRate() != format.rate )
        return false;

    AUDIO::TAudioSourcePtr source = OpenPossiblySlow( format, sourceDelayInMs, 1024 );
    if ( source.IsNULL() )
        return false;
    AUDIO::CAudioStream* stream = GUCEF_NEW AUDIO::CAudioStream( source, 200, streamLoops, true );
    AUDIO::TAudioSourcePtr streamPtr( stream );
    AUDIO::TAudioVoicePtr voice = engine.CreateVoice( streamPtr );
    if ( voice.IsNULL() )
        return false;
    voice->SetLoopCount( voiceLoops );
    CORE::CFutureResult started = engine.Start( voice );
    if ( started.HasNoFuture() )
        return false;

    played.ended = RenderUntilEnded( engine, voice, true );
    played.underruns = stream->GetUnderrunCount();
    CopyHeard( engine, played.heard );
    return played.ended;
}

/*-------------------------------------------------------------------------*/

/** A thread that reads a stream to its end */
class CStreamReader : public MT::CActiveObject
{
    public:

    CStreamReader( AUDIO::CAudioStream& stream, bool fillByHand )
        : MT::CActiveObject(), m_stream( stream ), m_fillByHand( fillByHand ), m_audio(), m_isOk( false ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_isOk = ReadStreamToEnd( m_stream, m_audio, m_fillByHand, 500 );
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

    AUDIO::CAudioStream& m_stream;
    bool m_fillByHand;
    AUDIO::CAudioBuffer m_audio;
    bool m_isOk;
    MT::CTAtomic< UInt32 > done;
};

/** A thread that calls Fill() of streams over and over, next to the read ahead task and the readers */
class CFillThread : public MT::CActiveObject
{
    public:

    CFillThread( AUDIO::CAudioStream** streams, UInt32 count )
        : MT::CActiveObject(), m_streams( streams ), m_count( count ), stop( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == stop.Load() )
        {
            for ( UInt32 i=0; i<m_count; ++i )
                m_streams[ i ]->Fill();
            MT::ThreadDelay( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    AUDIO::CAudioStream** m_streams;
    UInt32 m_count;
    MT::CTAtomic< UInt32 > stop;
};

/*-------------------------------------------------------------------------*/

const UInt32 StreamsPerPlugin = 4;

/**
 *  Four different files of one codec, four streams of it: two with the read ahead task and two that their reading thread
 *  fills by itself, one more thread that fills the streams of the task as well. Every stream gives what a plain decode
 *  of its file gives. 'maxOverlap' is how many reads of the plugin were inside it at the same time.
 */
bool
StreamsOfOnePlugin( const char* kind, TCodecPtr codec, Int32& maxOverlap )
{
    SFormat files[ StreamsPerPlugin ];
    for ( UInt32 i=0; i<StreamsPerPlugin; ++i )
        if ( !MakeFile( kind, codec, 30000 + 1000 * i, 11 + i, files[ i ] ) )
            return false;

    COverlapCounter counter;
    COverlapInput* inputs[ StreamsPerPlugin ];
    AUDIO::CAudioStream* streams[ StreamsPerPlugin ];
    AUDIO::TAudioSourcePtr holders[ StreamsPerPlugin ];
    CStreamReader* readers[ StreamsPerPlugin ];
    for ( UInt32 i=0; i<StreamsPerPlugin; ++i )
    {
        inputs[ i ] = GUCEF_NEW COverlapInput( files[ i ].file, counter );
        TInputPtr owner( inputs[ i ] );
        AUDIO::TAudioSourcePtr source;
        if ( !codec->OpenSource( owner, source ) )
            return false;
        streams[ i ] = GUCEF_NEW AUDIO::CAudioStream( source, 100, 1, i < 2 );
        holders[ i ] = AUDIO::TAudioSourcePtr( streams[ i ] );
        inputs[ i ]->Arm();
        readers[ i ] = GUCEF_NEW CStreamReader( *streams[ i ], i >= 2 );
    }
    CFillThread filler( streams, 2 );

    bool isOk = filler.Activate();
    for ( UInt32 i=0; i<StreamsPerPlugin; ++i )
        isOk = readers[ i ]->Activate() && isOk;
    for ( UInt32 i=0; i<StreamsPerPlugin; ++i )
    {
        isOk = readers[ i ]->WaitForDone( 60000 ) && isOk;
        readers[ i ]->Deactivate( false, true );
    }
    filler.stop.Store( 1 );
    filler.Deactivate( false, true );

    for ( UInt32 i=0; i<StreamsPerPlugin; ++i )
    {
        isOk = readers[ i ]->m_isOk && SameBytes( readers[ i ]->m_audio, files[ i ].reference ) && isOk;
        GUCEF_DELETE readers[ i ];
    }
    maxOverlap = counter.GetMaxInside();
    return isOk;
}

/*-------------------------------------------------------------------------*/

/** Playing on, after seeks of the render side, gives what a second source gives that goes to the same places */
bool
SeekWhileTheTaskRuns( SFormat& format )
{
    AUDIO::TAudioSourcePtr source;
    AUDIO::TAudioSourcePtr twin;
    if ( !OpenSource( format.codec, format.file, source ) || !OpenSource( format.codec, format.file, twin ) )
        return false;
    AUDIO::CAudioStream stream( source, 100, 1, true );
    const UInt32 length = format.reference.GetFrameCount();
    const UInt32 frameBytes = format.reference.GetFormat().GetBytesPerFrame();
    const UInt32 wanted = 800;
    CORE::CDynamicBuffer fromStream( wanted * frameBytes, true );
    CORE::CDynamicBuffer fromTwin( wanted * frameBytes, true );
    CRandom random( 4242 );
    bool isOk = true;

    for ( UInt32 i=0; i<40 && isOk; ++i )
    {
        // read on for a while first, so that the task is busy reading ahead when the seek comes
        if ( 0 == i % 3 )
            ReadSome( stream, fromStream.GetBufferPtr(), 100 + random.Below( 300 ), frameBytes );

        UInt64 target = random.Below( length );
        if ( 10 == i )
            target = length - 300;                          // the last frames: a short read, then the end
        if ( 11 == i )
            target = 0;
        const UInt32 seeks = 3 == i % 4 ? 3 : 1;            // seeks in a row without reading: the last one counts
        for ( UInt32 s=0; s<seeks; ++s )
        {
            if ( s + 1 < seeks )
                isOk = stream.SeekToFrame( random.Below( length ) ) && isOk;
            else
                isOk = stream.SeekToFrame( target ) && isOk;
        }
        isOk = target == stream.GetPositionInFrames() && isOk;

        const UInt32 got = ReadSome( stream, fromStream.GetBufferPtr(), wanted, frameBytes );
        isOk = twin->SeekToFrame( target ) && isOk;
        const UInt32 gotTwin = ReadSome( *twin, fromTwin.GetBufferPtr(), wanted, frameBytes );
        const UInt32 expectedFrames = (UInt32) ( length - target < wanted ? length - target : wanted );
        isOk = got == expectedFrames && got == gotTwin && isOk;
        isOk = target + got == stream.GetPositionInFrames() && isOk;
        if ( format.settlesAfterSeek )
        {
            // not the same samples as a second source gives: the noise that Opus fills in depends on where the decoder has been. The level has to be about that of the file there
            if ( got > 100 )
            {
                const AUDIO::CAudioBuffer heard( format.reference.GetFormat(), fromStream.GetConstBufferPtr(), got );
                Float64 energyHeard = 1.0e-20, energyReference = 1.0e-20;
                for ( UInt32 f=0; f<got; ++f )
                    for ( UInt32 c=0; c<format.reference.GetFormat().GetChannelCount(); ++c )
                    {
                        const Float64 h = heard.GetSampleAsFloat( f, c );
                        const Float64 r = format.reference.GetSampleAsFloat( (UInt32) target + f, c );
                        energyHeard += h * h;
                        energyReference += r * r;
                    }
                isOk = fabs( 10.0 * log10( energyHeard / energyReference ) ) < 3.0 && isOk;
            }
        }
        else
        {
            isOk = 0 == memcmp( fromStream.GetConstBufferPtr(), fromTwin.GetConstBufferPtr(), (size_t) got * frameBytes ) && isOk;
            if ( format.exactSeek && got > 0 )
                isOk = 0 == memcmp( fromStream.GetConstBufferPtr(), format.reference.GetDataPtr( (UInt32) target ), (size_t) got * frameBytes ) && isOk;
        }
        if ( !isOk )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: seek " << i << " to " << target << " gave " << got << " frames, the twin " << gotTwin << ", expected " << expectedFrames );
    }
    return isOk;
}

/*-------------------------------------------------------------------------*/

/** The streams are destroyed at any moment of their decoding, also with a seek on its way, and the task goes on to serve the next stream */
bool
TeardownWhileDecoding( SFormat& format )
{
    CRandom random( 99 );
    for ( UInt32 i=0; i<30; ++i )
    {
        AUDIO::TAudioSourcePtr source;
        if ( !OpenSource( format.codec, format.file, source ) )
            return false;
        AUDIO::CAudioStream* stream = GUCEF_NEW AUDIO::CAudioStream( source, 100, 0, true );
        source = AUDIO::TAudioSourcePtr();
        CORE::CDynamicBuffer buffer( 2048 * format.reference.GetFormat().GetBytesPerFrame(), true );
        stream->ReadFrames( buffer.GetBufferPtr(), 2048 );              // frees blocks: the task has something to read
        if ( 0 == i % 2 )
            stream->SeekToFrame( random.Below( format.reference.GetFrameCount() ) );
        MT::ThreadDelay( random.Below( 4 ) );
        GUCEF_DELETE stream;
    }

    // the task still serves the next stream
    AUDIO::TAudioSourcePtr source;
    if ( !OpenSource( format.codec, format.file, source ) )
        return false;
    AUDIO::CAudioStream stream( source, 100, 1, true );
    AUDIO::CAudioBuffer got;
    return ReadStreamToEnd( stream, got, false ) && SameBytes( got, format.reference );
}

/*-------------------------------------------------------------------------*/

/** Shuts the engine down while a voice plays a stream that is decoding, with the engine and the voice released afterwards in either order */
bool
ShutdownWhilePlayingStream( SFormat& format, bool shutdownFirst )
{
    for ( UInt32 round=0; round<3; ++round )
    {
        AUDIO::CAudioEngine* engine = GUCEF_NEW AUDIO::CAudioEngine();
        if ( !engine->Initialize( EngineParams( format.rate, true ) ) )
            return false;
        AUDIO::TAudioSourcePtr source = OpenPossiblySlow( format, 2, 2048 );
        if ( source.IsNULL() )
            return false;
        AUDIO::TAudioSourcePtr streamPtr( GUCEF_NEW AUDIO::CAudioStream( source, 200, 0, true ) );
        source = AUDIO::TAudioSourcePtr();
        AUDIO::TAudioVoicePtr voice = engine->CreateVoice( streamPtr );
        if ( voice.IsNULL() )
            return false;
        engine->Start( voice );
        RenderBlocks( *engine, 20 + 10 * round, true );
        if ( shutdownFirst )
        {
            engine->Shutdown();
            streamPtr = AUDIO::TAudioSourcePtr();
            voice = AUDIO::TAudioVoicePtr();
            GUCEF_DELETE engine;
        }
        else
        {
            streamPtr = AUDIO::TAudioSourcePtr();
            GUCEF_DELETE engine;                // the voice still holds the stream, so the stream outlives the engine here
            voice = AUDIO::TAudioVoicePtr();
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  A stream that loops forever keeps playing in order over a reopen of the device. Without a change of rate the audio
 *  before and after is the file in order, nothing lost or repeated. 'async' reopens on a task and polls with Update().
 */
bool
StreamSurvivesReopen( SFormat& format, bool changeRate, bool async )
{
    const bool threaded = !async;
    AUDIO::CAudioEngine engine;
    if ( !engine.Initialize( EngineParams( format.rate, threaded ) ) )
        return false;
    AUDIO::TAudioSourcePtr source = OpenPossiblySlow( format, 0, 0 );
    if ( source.IsNULL() )
        return false;
    AUDIO::CAudioStream* stream = GUCEF_NEW AUDIO::CAudioStream( source, 200, 0, true );
    AUDIO::TAudioSourcePtr streamPtr( stream );
    AUDIO::TAudioVoicePtr voice = engine.CreateVoice( streamPtr );
    if ( voice.IsNULL() || engine.Start( voice ).HasNoFuture() )
        return false;

    const UInt32 length = format.reference.GetFrameCount();
    const UInt32 halfBlocks = length / 2 / 256;
    RenderBlocks( engine, halfBlocks, threaded );
    TSamples before;
    CopyHeard( engine, before );
    const UInt64 positionBefore = voice->GetPositionInFrames();

    AUDIO::CAudioEngineParams next = engine.GetParams();
    next.GetDeviceParams().SetBufferSizeInFrames( 512 );
    if ( changeRate )
        next.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( format.rate == 48000 ? 44100 : 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
    AUDIO::TAudioDevicePtr oldDevice = engine.GetDevice();
    if ( async )
    {
        CORE::CFutureResult reopened = engine.ReopenDeviceAsync( next );
        for ( UInt32 waited=0; waited<10000 && !reopened.IsReady(); ++waited )
        {
            engine.Update();
            MT::ThreadDelay( 1 );
        }
        if ( !reopened.IsReady() || reopened.HasNoFuture() )
            return false;
    }
    else
    if ( !engine.ReopenDevice( next ) )
        return false;
    if ( engine.GetDevice() == oldDevice || 1 != engine.GetActiveVoiceCount() || AUDIO::AUDIOVOICESTATE_PLAYING != voice->GetState() )
        return false;

    RenderBlocks( engine, length * 3 / 4 / 256 + 4, threaded );
    TSamples after;
    CopyHeard( engine, after );
    if ( voice->GetPositionInFrames() <= positionBefore || AUDIO::AUDIOVOICESTATE_PLAYING != voice->GetState() )
        return false;

    if ( changeRate )
    {
        // resampled: only that it goes on sounding
        Float32 peak = 0.0f;
        for ( size_t i=0; i<after.size(); ++i )
            peak = fabsf( after[ i ] ) > peak ? fabsf( after[ i ] ) : peak;
        return peak > 0.05f && engine.GetMixFormat().GetSampleRate() != format.rate;
    }

    TSamples all( before );
    all.insert( all.end(), after.begin(), after.end() );
    return SameWhenSilenceIsLeftOut( all, format.reference, 3, true );
}


/*-------------------------------------------------------------------------*/

bool
PlainOnTask( SFormat& format )
{
    AUDIO::TAudioSourcePtr source;
    if ( !OpenSource( format.codec, format.file, source ) )
        return false;
    AUDIO::CAudioStream stream( source, 100, 1, true );
    AUDIO::CAudioBuffer got;
    return ReadStreamToEnd( stream, got, false ) && SameBytes( got, format.reference );
}

/*-------------------------------------------------------------------------*/

/** The voice loops three times: every restart is a seek of the render side to frame 0 while the read ahead task runs */
bool
VoiceLoopsThroughEngine( SFormat& format )
{
    SPlayed played;
    return PlayThroughEngine( format, 1, 3, 0, played ) && SameWhenSilenceIsLeftOut( played.heard, format.reference, 3, false );
}

/** The stream loops three times by itself: the read ahead continues at the start, the voice plays once */
bool
StreamLoopsThroughEngine( SFormat& format )
{
    SPlayed played;
    return PlayThroughEngine( format, 3, 1, 0, played ) && SameWhenSilenceIsLeftOut( played.heard, format.reference, 3, false );
}

/** A source that reads slower than the render side consumes: silence for the missing parts, an underrun count, nothing lost, repeated or out of order */
bool
SlowSourceThroughEngine( SFormat& format )
{
    SPlayed played;
    const bool isOk = PlayThroughEngine( format, 1, 1, 12, played ) && SameWhenSilenceIsLeftOut( played.heard, format.reference, 1, false );
    if ( isOk && 0 == played.underruns )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: no underrun was counted for " << format.name );
    return isOk && played.underruns > 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioStreamingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING audio streaming through the codec plugins TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Streaming" );

    // what another suite left loaded is not ours to unload
    const bool ownFlac = FindCodec( "flac" ).IsNULL();
    const bool ownOgg = FindCodec( "ogg" ).IsNULL();
    const bool ownMp3 = FindCodec( "mp3" ).IsNULL();
    const bool ownAu = FindCodec( "au" ).IsNULL();
    const bool hasFlac = LoadPlugin( "audiocodecpluginFLAC", "audiostreamingflac", "flac" );
    const bool hasOgg = LoadPlugin( "audiocodecpluginOGG", "audiostreamingogg", "ogg" );
    const bool hasMp3 = LoadPlugin( "audiocodecpluginMP3", "audiostreamingmp3", "mp3" );
    const bool hasAu = LoadPlugin( "audiocodecpluginAU", "audiostreamingau", "au" );

    SFormatSet* shortSet = GUCEF_NEW SFormatSet();        // 1.5 s of audio per format: loops, slow sources, reopen
    SFormatSet* longSet = GUCEF_NEW SFormatSet();         // 3 s: seeks
    BuildFormats( *shortSet, 66150, 5, false );
    BuildFormats( *longSet, 132300, 6, false );

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: The plugins are there and every format has a file for the tests" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    GUCEF_TESTFW_SKIP_IF( !hasFlac && !hasOgg && !hasMp3, "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( hasFlac && hasOgg && hasMp3 && hasAu );
        ASSERT_TRUE( shortSet->count >= 3 && longSet->count == shortSet->count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: Every codec through a stream with the read ahead task gives what a plain decode gives" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, PlainOnTask( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: Seeks of the render side while the read ahead task runs land on the right frame, also seeks in a row, the last frames and frame 0" )
    GUCEF_TESTFW_SKIP_IF( 0 == longSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *longSet, SeekWhileTheTaskRuns( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A voice that loops three times restarts its stream by seek in the threaded engine without losing, repeating or reordering a frame" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, VoiceLoopsThroughEngine( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A stream that loops three times by itself plays seamlessly through the threaded engine" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, StreamLoopsThroughEngine( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A source slower than the playback gives silence and underruns, loses nothing, and the voice ends only at the end of the source" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, SlowSourceThroughEngine( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: Four streams of one plugin at once, two on the task and two filled by their reading threads, each gives its own file; the plugin lock or per source threading holds" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        const char* kinds[ 5 ] = { "flac", "vorbis", "opus", "mp3", "au" };
        const char* types[ 5 ] = { "flac", "ogg", "opus", "mp3", "au" };
        for ( UInt32 k=0; k<5; ++k )
        {
            TCodecPtr codec = FindCodec( types[ k ] );
            if ( codec.IsNULL() )
                continue;
            if ( 0 == strcmp( kinds[ k ], "opus" ) )
            {
                CORE::CDynamicBuffer probe;
                if ( !Encode( codec, MakeTones( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_INT16 ), 4800, 500.0, 0.5 ), probe ) )
                    continue;
            }
            Int32 maxOverlap = 0;
            const bool isOk = StreamsOfOnePlugin( kinds[ k ], codec, maxOverlap );
            const bool perSource = GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == GetThreadingModel( types[ k ] );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioStreaming: " << kinds[ k ] << ": streams decoded correctly " << isOk << ", reads of the plugin inside it at once: " << maxOverlap << ( perSource ? " (per source threading)" : " (one call at a time)" ) );
            ASSERT_TRUE( isOk );
            // a plugin that reads its whole input when it opens a stream (MP3) never reads the input while it decodes: nothing to overlap
            ASSERT_TRUE( 0 == maxOverlap || ( perSource ? maxOverlap >= 2 : 1 == maxOverlap ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A stream destroyed while the task decodes a slow source releases the source and the task goes on" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        SFormat& format = shortSet->items[ 0 ];
        MT::CTAtomic< UInt32 > destroyed( 0 );
        CRandom random( 31 );
        const UInt32 rounds = 12;
        for ( UInt32 i=0; i<rounds; ++i )
        {
            AUDIO::CAudioStream* stream = GUCEF_NEW AUDIO::CAudioStream( OpenPossiblySlow( format, 8, 1024, &destroyed ), 100, 0, true );
            CORE::CDynamicBuffer buffer( 4096 * format.reference.GetFormat().GetBytesPerFrame(), true );
            stream->ReadFrames( buffer.GetBufferPtr(), 4096 );          // frees blocks, so the slow task is busy reading
            MT::ThreadDelay( random.Below( 25 ) );
            const UInt64 start = MT::PrecisionTickCount();
            GUCEF_DELETE stream;
            const Float64 tookInMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            ASSERT_TRUE( tookInMs < 2000.0 );
            ASSERT_TRUE( i + 1 == destroyed.Load() );
        }
        ASSERT_TRUE( PlainOnTask( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: Streams of every codec destroyed at any moment of the decoding, with a seek on its way, leave the task working" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, TeardownWhileDecoding( format ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: The engine shut down or deleted while a voice plays a streaming codec source, in either order with the stream" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, ShutdownWhilePlayingStream( format, true ) && ShutdownWhilePlayingStream( format, false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A looping stream survives a synchronous device reopen in the threaded engine: the audio goes on in order" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, StreamSurvivesReopen( format, false, false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A looping stream survives an asynchronous device reopen of the cooperative engine: the audio goes on in order" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, StreamSurvivesReopen( format, false, true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: A looping stream survives a reopen with another rate, resampled, and keeps playing" )
    GUCEF_TESTFW_SKIP_IF( 0 == shortSet->count, "no codec plugin is there" );
    try
    {
        CHECK_EACH_FORMAT( *shortSet, StreamSurvivesReopen( format, true, false ) && StreamSurvivesReopen( format, true, true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_DELETE shortSet;
    GUCEF_DELETE longSet;

    GUCEF_TESTFW_TESTCASE( "AudioStreaming: Unloading the plugins takes the types away" )
    try
    {
        ASSERT_TRUE( !hasFlac || !ownFlac || UnloadPlugin( "audiostreamingflac", "flac" ) );
        ASSERT_TRUE( !hasOgg || !ownOgg || UnloadPlugin( "audiostreamingogg", "ogg" ) );
        ASSERT_TRUE( !hasMp3 || !ownMp3 || UnloadPlugin( "audiostreamingmp3", "mp3" ) );
        ASSERT_TRUE( !hasAu || !ownAu || UnloadPlugin( "audiostreamingau", "au" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
