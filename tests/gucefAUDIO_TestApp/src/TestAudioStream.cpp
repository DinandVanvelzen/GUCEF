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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioStream.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  A float source whose samples are the number of their frame, or a constant when one is given, so the order
 *  in which frames arrive can be checked. As a live source it only has the frames that were made available.
 */
class CCountingSource : public AUDIO::IAudioSource
{
    public:

    CCountingSource( UInt32 sampleRate, UInt32 channels, UInt64 length, bool seekable = true, bool live = false, Float32 constant = -1.0f )
        : m_format( sampleRate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), m_length( length ), m_position( 0 ), m_seekable( seekable ), m_live( live ), m_constant( constant ), available( 0 ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 limit = m_live ? ( available < m_length ? available : m_length ) : m_length;
        UInt32 frames = 0;
        Float32* out = static_cast< Float32* >( destination );
        while ( frames < frameCapacity && m_position < limit )
        {
            for ( UInt32 c=0; c<m_format.GetChannelCount(); ++c )
                *out++ = m_constant >= 0.0f ? m_constant : (Float32) m_position;
            ++m_position;
            ++frames;
        }
        return frames;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position >= m_length; }
    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_seekable; }

    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_seekable || frame > m_length )
            return false;
        m_position = frame;
        return true;
    }

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }
    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_length; }

    private:

    AUDIO::CAudioFormat m_format;
    UInt64 m_length;
    UInt64 m_position;
    bool m_seekable;
    bool m_live;
    Float32 m_constant;

    public:

    UInt64 available;   /**< live source: frames that exist so far */
};

AUDIO::IAudioSource::TAudioSourcePtr
Counting( UInt64 length, bool seekable = true )
{
    return AUDIO::IAudioSource::TAudioSourcePtr( GUCEF_NEW CCountingSource( 8000, 1, length, seekable ) );
}

/** Reads the stream to its end, filling it by hand before every read when asked; the frames as their numbers */
GUCEF::vector< UInt32 >
ReadAll( AUDIO::CAudioStream& stream, bool fillByHand, UInt32 maxFrames, UInt32 readSize = 300 )
{
    GUCEF::vector< UInt32 > frames;
    GUCEF::vector< Float32 > buffer( readSize * stream.GetFormat().GetChannelCount() );
    UInt32 idleRounds = 0;
    while ( !stream.IsEndOfStream() && frames.size() < maxFrames && idleRounds < 2000 )
    {
        if ( fillByHand )
            stream.Fill();
        const UInt32 got = stream.ReadFrames( &buffer[ 0 ], readSize );
        for ( UInt32 f=0; f<got; ++f )
            frames.push_back( (UInt32) buffer[ f * stream.GetFormat().GetChannelCount() ] );
        if ( 0 == got )
        {
            ++idleRounds;
            MT::PrecisionDelay( 2 );
        }
    }
    return frames;
}

bool
CountsUp( const GUCEF::vector< UInt32 >& frames, size_t from, size_t count, UInt32 firstValue )
{
    if ( from + count > frames.size() )
        return false;
    for ( size_t i=0; i<count; ++i )
        if ( frames[ from + i ] != firstValue + (UInt32) i )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioStreamTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioStream TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Stream" );

    GUCEF_TESTFW_TESTCASE( "AudioStream: Gives the whole source in order and then ends" )
    try
    {
        AUDIO::CAudioStream stream( Counting( 20000 ), 200, 1, false );
        ASSERT_TRUE( stream.GetFormat() == AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( stream.GetBufferedFrames() > 0 );                          // the start was read by the constructor
        ASSERT_TRUE( 20000 == stream.GetLengthInFrames() && stream.IsSeekable() );

        const GUCEF::vector< UInt32 > frames = ReadAll( stream, true, 100000 );
        ASSERT_TRUE( 20000 == frames.size() && CountsUp( frames, 0, 20000, 0 ) );
        ASSERT_TRUE( stream.IsEndOfStream() );
        ASSERT_TRUE( 20000 == stream.GetPositionInFrames() );
        ASSERT_TRUE( 0 == stream.GetUnderrunCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStream: Loops by itself without a gap, also forever" )
    try
    {
        AUDIO::CAudioStream three( Counting( 1000 ), 200, 3, false );
        const GUCEF::vector< UInt32 > frames = ReadAll( three, true, 100000, 333 );
        ASSERT_TRUE( 3000 == frames.size() );
        ASSERT_TRUE( CountsUp( frames, 0, 1000, 0 ) && CountsUp( frames, 1000, 1000, 0 ) && CountsUp( frames, 2000, 1000, 0 ) );
        ASSERT_TRUE( three.IsEndOfStream() && 0 == three.GetUnderrunCount() );

        AUDIO::CAudioStream forever( Counting( 700 ), 200, 0, false );
        ASSERT_TRUE( GUCEF_AUDIO_LENGTH_UNKNOWN == forever.GetLengthInFrames() );
        const GUCEF::vector< UInt32 > many = ReadAll( forever, true, 7000 );
        ASSERT_TRUE( many.size() >= 7000 && CountsUp( many, 6300, 700, 0 ) );
        ASSERT_FALSE( forever.IsEndOfStream() );

        // looping forever needs a source that can go back
        AUDIO::CAudioStream once( Counting( 700, false ), 200, 0, false );
        ASSERT_TRUE( 700 == ReadAll( once, true, 7000 ).size() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStream: A seek drops what was read ahead and continues at the new place" )
    try
    {
        AUDIO::CAudioStream stream( Counting( 20000 ), 200, 1, false );
        Float32 buffer[ 100 ];
        ASSERT_TRUE( 100 == stream.ReadFrames( buffer, 100 ) && 99.0f == buffer[ 99 ] );

        ASSERT_TRUE( stream.SeekToFrame( 15000 ) );
        ASSERT_TRUE( 15000 == stream.GetPositionInFrames() );
        ASSERT_TRUE( 0 == stream.GetBufferedFrames() );
        ASSERT_TRUE( 0 == stream.ReadFrames( buffer, 100 ) );                   // nothing read ahead for the new place yet
        ASSERT_FALSE( stream.IsEndOfStream() );

        const GUCEF::vector< UInt32 > rest = ReadAll( stream, true, 100000 );
        ASSERT_TRUE( 5000 == rest.size() && CountsUp( rest, 0, 5000, 15000 ) );
        ASSERT_FALSE( stream.SeekToFrame( 20001 ) );                             // beyond the end

        // a seek after the end starts it again, the way a voice loops
        ASSERT_TRUE( stream.IsEndOfStream() && stream.SeekToFrame( 0 ) && !stream.IsEndOfStream() );
        ASSERT_TRUE( 20000 == ReadAll( stream, true, 100000 ).size() );

        AUDIO::CAudioStream fixed( Counting( 1000, false ), 200, 1, false );
        ASSERT_FALSE( fixed.IsSeekable() || fixed.SeekToFrame( 10 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStream: When the read ahead falls behind it gives what it has, counts an underrun and does not end" )
    try
    {
        CCountingSource* live = GUCEF_NEW CCountingSource( 8000, 1, 10000, true, true );
        live->available = 500;
        AUDIO::CAudioStream stream( AUDIO::IAudioSource::TAudioSourcePtr( live ), 200, 1, false );
        Float32 buffer[ 1000 ];
        ASSERT_TRUE( 500 == stream.ReadFrames( buffer, 1000 ) );
        ASSERT_FALSE( stream.IsEndOfStream() );
        ASSERT_TRUE( 1 == stream.GetUnderrunCount() );

        live->available = 1200;     // the read ahead is the only one reading the source, this is safe while it does not run
        stream.Fill();
        ASSERT_TRUE( 700 == stream.ReadFrames( buffer, 1000 ) && 500.0f == buffer[ 0 ] && 1199.0f == buffer[ 699 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStream: Reads ahead by itself on a task of the streaming pool" )
    try
    {
        // a quarter second of buffer for half a minute of audio: the task has to keep up with the reading
        AUDIO::CAudioStream stream( Counting( 8000 * 30 ), 250, 1, true );
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( AUDIO::CAudioStream::StreamingThreadPoolName );
        ASSERT_TRUE( !pool.IsNULL() && pool->GetActiveNrOfDedicatedThreads() >= 1 );

        const GUCEF::vector< UInt32 > frames = ReadAll( stream, false, 8000 * 31, 800 );
        ASSERT_TRUE( 8000 * 30 == frames.size() && CountsUp( frames, 0, 8000 * 30, 0 ) );
        ASSERT_TRUE( stream.IsEndOfStream() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioStream: Plays through the engine like any source" )
    try
    {
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        params.GetDeviceParams().SetBufferSizeInFrames( 256 );
        params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );

        AUDIO::CAudioStream* stream = GUCEF_NEW AUDIO::CAudioStream( AUDIO::IAudioSource::TAudioSourcePtr( GUCEF_NEW CCountingSource( 48000, 1, 5000, true, false, 0.5f ) ), 100, 1, false );
        AUDIO::IAudioSource::TAudioSourcePtr source( stream );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( source );
        ASSERT_FALSE( voice.IsNULL() );
        CORE::CFutureResult started = engine.Start( voice );
        ASSERT_FALSE( started.HasNoFuture() );

        AUDIO::TAudioDevicePtr device = engine.GetDevice();
        AUDIO::CNullAudioDevice* null = ( device.IsNULL() ? GUCEF_NULL : device->AsNullDevice() );
        ASSERT_TRUE( GUCEF_NULL != null );
        for ( UInt32 i=0; i<30 && GUCEF_NULL != null; ++i )
        {
            stream->Fill();
            null->Render( 256 );
            engine.Update();
        }
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_ENDED == voice->GetState() );
        ASSERT_TRUE( 0 == stream->GetUnderrunCount() );
        const AUDIO::CAudioBuffer& heard = null->GetRenderedAudio();
        ASSERT_TRUE( heard.GetFrameCount() >= 5256 );
        ASSERT_TRUE( 0.5f == heard.GetSampleAsFloat( 0, 0 ) && 0.5f == heard.GetSampleAsFloat( 4999, 1 ) && 0.0f == heard.GetSampleAsFloat( 5000, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
