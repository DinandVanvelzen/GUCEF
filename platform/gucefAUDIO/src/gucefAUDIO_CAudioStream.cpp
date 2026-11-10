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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUDIO_CAudioStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioStream::StreamingThreadPoolName = "AudioStreaming";
const UInt32 CAudioStream::DefaultBufferDurationInMs;
const UInt32 CAudioStream::NoBlock;

namespace {

/** Frames per block: small enough to keep the read ahead fine grained, large enough to keep the overhead low */
const UInt32 BlockDurationInMs = 50;

/** How often the read ahead task looks at the streams */
const UInt32 ReadAheadIntervalInMs = 10;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The task that reads ahead for all streams that want it. It is made when the first such stream is made and
 *  ends when the module shuts down. The list lock is held while a stream is being filled, so a stream that
 *  leaves the list knows the task is not busy with it once it has left.
 */
class CAudioStreamReadAheadTask : public CORE::CTaskConsumer
{
    public:

    typedef CORE::CTSharedPtr< CAudioStreamReadAheadTask, MT::CMutex > TReadAheadTaskPtr;

    CAudioStreamReadAheadTask( void )
        : CORE::CTaskConsumer()
        , m_lock()
        , m_streams()
        , m_stopped( false )
    {GUCEF_TRACE;
    }

    virtual ~CAudioStreamReadAheadTask() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return "GUCEF::AUDIO::CAudioStreamReadAheadTask";
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        static const CORE::CString typeName = "GUCEF::AUDIO::CAudioStreamReadAheadTask";
        return typeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        if ( m_stopped )
            return true;
        for ( size_t i=0; i<m_streams.size(); ++i )
            m_streams[ i ]->Fill();
        RequestTaskCycleDelayInMs( ReadAheadIntervalInMs );
        return false;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    void Add( CAudioStream* stream )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        m_streams.push_back( stream );
    }

    void Remove( CAudioStream* stream )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_streams.size(); ++i )
        {
            if ( m_streams[ i ] == stream )
            {
                m_streams.erase( m_streams.begin() + i );
                return;
            }
        }
    }

    /** Once this returns the task does not touch any stream again */
    void Disconnect( void )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        m_stopped = true;
        m_streams.clear();
    }

    private:

    MT::CMutex m_lock;
    GUCEF::vector< CAudioStream* > m_streams;
    bool m_stopped;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

MT::CMutex g_readAheadTaskLock;
CAudioStreamReadAheadTask::TReadAheadTaskPtr g_readAheadTask;

/** The read ahead task, started when it is first needed */
CAudioStreamReadAheadTask::TReadAheadTaskPtr
GetReadAheadTask( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_readAheadTaskLock );
    if ( g_readAheadTask.IsNULL() )
    {
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CAudioStream::StreamingThreadPoolName );
        if ( pool.IsNULL() )
            return CAudioStreamReadAheadTask::TReadAheadTaskPtr();

        CAudioStreamReadAheadTask::TReadAheadTaskPtr task( GUCEF_NEW CAudioStreamReadAheadTask() );
        CORE::CFutureResult started = pool->StartTaskWithConsumer( task );
        if ( started.HasNoFuture() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioStream: The read ahead task could not be started on pool " << CAudioStream::StreamingThreadPoolName );
            return CAudioStreamReadAheadTask::TReadAheadTaskPtr();
        }
        g_readAheadTask = task;
    }
    return g_readAheadTask;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioStream::CAudioStream( const TAudioSourcePtr& source  ,
                            UInt32 bufferDurationInMs      ,
                            UInt32 loopCount               ,
                            bool readAheadOnTask           )
    : IAudioSource()
    , m_source( source )
    , m_format()
    , m_bytesPerFrame( 0 )
    , m_blockFrames( 0 )
    , m_sourceLength( GUCEF_AUDIO_LENGTH_UNKNOWN )
    , m_loopCount( loopCount )
    , m_seekable( false )
    , m_readAheadOnTask( false )
    , m_blocks()
    , m_filledBlocks( GUCEF_NULL )
    , m_freeBlocks( GUCEF_NULL )
    , m_requestedGeneration( 0 )
    , m_seekTarget( 0 )
    , m_underruns( 0 )
    , m_bufferedFrames( 0 )
    , m_generation( 0 )
    , m_currentBlock( NoBlock )
    , m_currentOffset( 0 )
    , m_position( 0 )
    , m_atEnd( true )
    , m_fillLock()
    , m_fillGeneration( 0 )
    , m_spareBlock( NoBlock )
    , m_playsDone( 0 )
    , m_framesThisPlay( 0 )
    , m_fillAtEnd( true )
{GUCEF_TRACE;

    if ( m_source.IsNULL() || !m_source->GetFormat().IsValid() )
        return;

    m_format = m_source->GetFormat();
    m_bytesPerFrame = m_format.GetBytesPerFrame();
    m_sourceLength = m_source->GetLengthInFrames();
    m_seekable = m_source->IsSeekable();
    if ( 0 == m_loopCount && !m_seekable )
        m_loopCount = 1;    // looping forever needs the source to go back to its start

    // The blocks, all of them made now: the render path never allocates
    m_blockFrames = ( m_format.GetSampleRate() * BlockDurationInMs ) / 1000;
    if ( m_blockFrames < 256 )
        m_blockFrames = 256;
    UInt32 blockCount = ( ( m_format.GetSampleRate() * ( bufferDurationInMs > 0 ? bufferDurationInMs : DefaultBufferDurationInMs ) ) / 1000 ) / m_blockFrames + 1;
    if ( blockCount < 2 )
        blockCount = 2;

    m_blocks.resize( blockCount );
    for ( UInt32 i=0; i<blockCount; ++i )
        m_blocks[ i ].data.resize( (size_t) m_blockFrames * m_bytesPerFrame );

    // The queues hold all blocks at once. Here, before anything else uses them, this thread can stand in for both sides
    m_filledBlocks = GUCEF_NEW MT::CTSpscQueue< UInt32 >( blockCount + 1 );
    m_freeBlocks = GUCEF_NEW MT::CTSpscQueue< UInt32 >( blockCount + 1 );
    for ( UInt32 i=0; i<blockCount; ++i )
        m_freeBlocks->Push( i );

    m_atEnd = false;
    m_fillAtEnd = false;

    // The start is read now so playback can begin at once
    Fill();

    if ( readAheadOnTask )
    {
        CAudioStreamReadAheadTask::TReadAheadTaskPtr task = GetReadAheadTask();
        if ( !task.IsNULL() )
        {
            task->Add( this );
            m_readAheadOnTask = true;
        }
    }
}

/*-------------------------------------------------------------------------*/

CAudioStream::~CAudioStream()
{GUCEF_TRACE;

    if ( m_readAheadOnTask )
    {
        CAudioStreamReadAheadTask::TReadAheadTaskPtr task;
        {
            MT::CScopeMutex lock( g_readAheadTaskLock );
            task = g_readAheadTask;
        }
        if ( !task.IsNULL() )
            task->Remove( this );
    }

    GUCEF_DELETE m_filledBlocks;
    m_filledBlocks = GUCEF_NULL;
    GUCEF_DELETE m_freeBlocks;
    m_freeBlocks = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAudioStream::StopReadAheadTask( void )
{GUCEF_TRACE;

    CAudioStreamReadAheadTask::TReadAheadTaskPtr task;
    {
        MT::CScopeMutex lock( g_readAheadTaskLock );
        task = g_readAheadTask;
        g_readAheadTask.Unlink();
    }
    if ( !task.IsNULL() )
    {
        task->Disconnect();
        task->RequestTaskToStop( false );
    }
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioStream::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioStream::ReadFrames( void* destination, UInt32 frameCapacity )
{
    // no trace macro: this runs where the audio is mixed

    UInt8* out = static_cast< UInt8* >( destination );
    UInt32 done = 0;
    while ( done < frameCapacity && !m_atEnd )
    {
        if ( NoBlock == m_currentBlock )
        {
            UInt32 index = NoBlock;
            if ( !m_filledBlocks->Pop( index ) )
                break;
            if ( m_blocks[ index ].generation != m_generation )
            {
                // read before a seek: not wanted any more
                m_bufferedFrames.FetchAdd( (UInt32) ( 0 - m_blocks[ index ].frameCount ), MT::MEMORDER_RELAXED );
                m_freeBlocks->Push( index );
                continue;
            }
            m_currentBlock = index;
            m_currentOffset = 0;
        }

        SBlock& block = m_blocks[ m_currentBlock ];
        UInt32 frames = block.frameCount - m_currentOffset;
        if ( frames > frameCapacity - done )
            frames = frameCapacity - done;
        if ( frames > 0 )
        {
            memcpy( out + (size_t) done * m_bytesPerFrame, &block.data[ (size_t) m_currentOffset * m_bytesPerFrame ], (size_t) frames * m_bytesPerFrame );
            m_currentOffset += frames;
            done += frames;
            m_bufferedFrames.FetchAdd( (UInt32) ( 0 - frames ), MT::MEMORDER_RELAXED );
        }

        if ( m_currentOffset >= block.frameCount )
        {
            const bool last = block.isLast;
            m_freeBlocks->Push( m_currentBlock );
            m_currentBlock = NoBlock;
            if ( last )
                m_atEnd = true;
        }
    }

    m_position += done;
    if ( done < frameCapacity && !m_atEnd && !m_source.IsNULL() )
        m_underruns.FetchAdd( 1, MT::MEMORDER_RELAXED );
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CAudioStream::IsEndOfStream( void ) const
{
    return m_atEnd;
}

/*-------------------------------------------------------------------------*/

bool
CAudioStream::IsRealtimeSafe( void ) const
{
    // the render side only copies what the read ahead task prepared
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioStream::IsSeekable( void ) const
{
    return m_seekable;
}

/*-------------------------------------------------------------------------*/

bool
CAudioStream::GetMetaData( CORE::CValueList& metaData ) const
{GUCEF_TRACE;

    if ( m_source.IsNULL() )
        return false;

    // the read ahead task uses the source on its own thread, so it is only touched under the fill lock
    MT::CScopeMutex lock( m_fillLock );
    return m_source->GetMetaData( metaData );
}

/*-------------------------------------------------------------------------*/

bool
CAudioStream::SeekToFrame( UInt64 frame )
{
    // no trace macro: a voice that loops calls this where the audio is mixed

    if ( !m_seekable || ( GUCEF_AUDIO_LENGTH_UNKNOWN != m_sourceLength && frame > m_sourceLength ) )
        return false;

    // Drop what was read ahead. The read ahead learns of the seek through the generation, anything it is
    // still making for the old one is dropped when it arrives
    ++m_generation;
    if ( NoBlock != m_currentBlock )
    {
        m_bufferedFrames.FetchAdd( (UInt32) ( 0 - ( m_blocks[ m_currentBlock ].frameCount - m_currentOffset ) ), MT::MEMORDER_RELAXED );
        m_freeBlocks->Push( m_currentBlock );
        m_currentBlock = NoBlock;
    }
    UInt32 index = NoBlock;
    while ( m_filledBlocks->Pop( index ) )
    {
        m_bufferedFrames.FetchAdd( (UInt32) ( 0 - m_blocks[ index ].frameCount ), MT::MEMORDER_RELAXED );
        m_freeBlocks->Push( index );
    }

    m_seekTarget.Store( frame, MT::MEMORDER_RELAXED );
    m_requestedGeneration.Store( m_generation, MT::MEMORDER_RELEASE );
    m_position = frame;
    m_atEnd = false;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioStream::GetPositionInFrames( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioStream::GetLengthInFrames( void ) const
{GUCEF_TRACE;

    return 0 == m_loopCount ? GUCEF_AUDIO_LENGTH_UNKNOWN : m_sourceLength;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioStream::FillBlock( SBlock& block, bool& ended )
{GUCEF_TRACE;

    ended = false;
    UInt32 filled = 0;
    while ( filled < m_blockFrames )
    {
        const UInt32 got = m_source->ReadFrames( &block.data[ (size_t) filled * m_bytesPerFrame ], m_blockFrames - filled );
        filled += got;
        m_framesThisPlay += got;
        if ( got > 0 )
            continue;

        if ( !m_source->IsEndOfStream() )
            break;                  // a live source with nothing right now, there will be more later

        ++m_playsDone;
        const bool again = ( 0 == m_loopCount || m_playsDone < m_loopCount ) && m_framesThisPlay > 0;
        if ( again && m_source->SeekToFrame( 0 ) )
        {
            m_framesThisPlay = 0;
            continue;
        }
        ended = true;
        break;
    }
    return filled;
}

/*-------------------------------------------------------------------------*/

void
CAudioStream::Fill( void )
{GUCEF_TRACE;

    if ( m_source.IsNULL() || 0 == m_blockFrames )
        return;

    MT::CScopeMutex lock( m_fillLock );

    const UInt32 generation = m_requestedGeneration.Load( MT::MEMORDER_ACQUIRE );
    if ( generation != m_fillGeneration )
    {
        const UInt64 target = m_seekTarget.Load( MT::MEMORDER_RELAXED );
        if ( !m_source->SeekToFrame( target ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioStream: The source could not go to frame " << target );
        m_fillGeneration = generation;
        m_fillAtEnd = false;
        m_framesThisPlay = target;
        if ( 0 == target )
            m_playsDone = 0;
    }

    while ( !m_fillAtEnd )
    {
        UInt32 index = m_spareBlock;
        m_spareBlock = NoBlock;
        if ( NoBlock == index && !m_freeBlocks->Pop( index ) )
            break;                  // read ahead as far as the buffer goes

        SBlock& block = m_blocks[ index ];
        bool ended = false;
        const UInt32 filled = FillBlock( block, ended );
        if ( 0 == filled && !ended )
        {
            m_spareBlock = index;   // nothing to give right now, the block is used next time
            break;
        }

        block.generation = generation;
        block.frameCount = filled;
        block.isLast = ended;
        m_bufferedFrames.FetchAdd( filled, MT::MEMORDER_RELAXED );
        m_filledBlocks->Push( index );

        if ( ended )
            m_fillAtEnd = true;

        // a seek came in while reading: the next turn starts at the new place
        if ( m_requestedGeneration.Load( MT::MEMORDER_ACQUIRE ) != generation )
            break;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioStream::GetUnderrunCount( void ) const
{GUCEF_TRACE;

    return m_underruns.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioStream::GetBufferedFrames( void ) const
{GUCEF_TRACE;

    return m_bufferedFrames.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
