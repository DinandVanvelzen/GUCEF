/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#include "gucefVIDEO_CVideoDemuxer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The sound of one track of a demuxed video, as an audio source
 */
class CVideoAudioTrackSource : public AUDIO::IAudioSource
{
    public:

    CVideoAudioTrackSource( const TVideoDemuxerPtr& demuxer, UInt32 track, const AUDIO::CAudioFormat& format )
        : AUDIO::IAudioSource()
        , m_demuxer( demuxer )
        , m_track( track )
        , m_format( format )
        , m_position( 0 )
        , m_ended( false )
    {GUCEF_TRACE;

        m_demuxer->SubscribeTrack( m_track, true );
    }

    virtual ~CVideoAudioTrackSource() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_demuxer->SubscribeTrack( m_track, false );
    }

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_format;
    }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_ended || GUCEF_NULL == destination || 0 == frameCapacity )
            return 0;

        const UInt32 read = m_demuxer->ReadAudio( m_track, destination, frameCapacity, m_ended );
        m_position += read;
        return read;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_ended;
    }

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_position;
    }

    private:

    TVideoDemuxerPtr m_demuxer;
    UInt32 m_track;
    AUDIO::CAudioFormat m_format;
    UInt64 m_position;
    bool m_ended;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoDemuxer::SAudioQueue::SAudioQueue( void )
    : data()
    , readOffset( 0 )
    , frameSize( 0 )
    , subscribers( 0 )
{GUCEF_TRACE;

    data.SetAutoEnlarge( true );
    data.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
}

/*-------------------------------------------------------------------------*/

CVideoDemuxer::CVideoDemuxer( const TVideoSourcePtr& source )
    : IVideoAudioSink()
    , m_lock()
    , m_source( source )
    , m_format()
    , m_decodeFrame()
    , m_queuedFrames()
    , m_freeFrames()
    , m_audioQueues()
    , m_consumesPictures( true )
    , m_maxQueuedFrames( DefaultMaxQueuedFrames )
{GUCEF_TRACE;

    if ( !m_source.IsNULL() )
    {
        m_format = m_source->GetFormat();
        const UInt32 trackCount = m_source->GetAudioTrackCount();
        m_audioQueues.resize( trackCount );
        for ( UInt32 track=0; track<trackCount; ++track )
        {
            AUDIO::CAudioFormat format;
            if ( m_source->GetAudioTrackFormat( track, format ) )
                m_audioQueues[ track ].frameSize = format.GetBytesPerFrame();
        }
    }
}

/*-------------------------------------------------------------------------*/

CVideoDemuxer::~CVideoDemuxer()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TFrameList::iterator i = m_queuedFrames.begin(); i != m_queuedFrames.end(); ++i )
        GUCEF_DELETE (*i);
    m_queuedFrames.clear();
    for ( TFrameVector::iterator i = m_freeFrames.begin(); i != m_freeFrames.end(); ++i )
        GUCEF_DELETE (*i);
    m_freeFrames.clear();
}

/*-------------------------------------------------------------------------*/

const CVideoFormat&
CVideoDemuxer::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoDemuxer::GetAudioTrackCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_audioQueues.size();
}

/*-------------------------------------------------------------------------*/

bool
CVideoDemuxer::GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_source.IsNULL() && m_source->GetAudioTrackFormat( track, format );
}

/*-------------------------------------------------------------------------*/

CVideoFrame*
CVideoDemuxer::AcquireQueueFrame( void )
{GUCEF_TRACE;

    if ( m_freeFrames.empty() )
        return GUCEF_NEW CVideoFrame();

    CVideoFrame* frame = m_freeFrames.back();
    m_freeFrames.pop_back();
    return frame;
}

/*-------------------------------------------------------------------------*/

bool
CVideoDemuxer::DecodeOne( bool queuePicture )
{GUCEF_TRACE;

    if ( m_source.IsNULL() || m_source->IsEndOfStream() )
        return false;

    if ( !m_source->DecodeNextFrame( m_decodeFrame, this ) )
        return false;

    if ( queuePicture )
    {
        CVideoFrame* queued = AcquireQueueFrame();
        *queued = m_decodeFrame;
        m_queuedFrames.push_back( queued );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVideoDemuxer::OnVideoAudio( UInt32 track, const void* frames, UInt32 frameCount )
{GUCEF_TRACE;

    // called by the source inside DecodeOne(), so the lock is held
    if ( track >= m_audioQueues.size() )
        return;

    SAudioQueue& queue = m_audioQueues[ track ];
    if ( 0 == queue.subscribers || 0 == queue.frameSize )
        return;

    queue.data.Append( frames, frameCount * queue.frameSize );
}

/*-------------------------------------------------------------------------*/

bool
CVideoDemuxer::ReadFrame( CVideoFrame& frame )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_queuedFrames.empty() )
    {
        CVideoFrame* queued = m_queuedFrames.front();
        m_queuedFrames.erase( m_queuedFrames.begin() );
        frame = *queued;
        m_freeFrames.push_back( queued );
        return true;
    }

    if ( !DecodeOne( false ) )
        return false;

    frame = m_decodeFrame;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoDemuxer::SkipFrame( bool* paletteChanged )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_queuedFrames.empty() )
    {
        CVideoFrame* queued = m_queuedFrames.front();
        m_queuedFrames.erase( m_queuedFrames.begin() );
        if ( GUCEF_NULL != paletteChanged )
            *paletteChanged = queued->IsPaletteChanged();
        m_freeFrames.push_back( queued );
        return true;
    }
    if ( !DecodeOne( false ) )
        return false;
    if ( GUCEF_NULL != paletteChanged )
        *paletteChanged = m_decodeFrame.IsPaletteChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoDemuxer::ReadAudio( UInt32 track, void* destination, UInt32 frameCapacity, bool& ended )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    ended = false;
    if ( track >= m_audioQueues.size() || 0 == m_audioQueues[ track ].frameSize )
    {
        ended = true;
        return 0;
    }

    // decode until there is enough sound, the video ends, or the pictures wait for their consumer
    SAudioQueue& queue = m_audioQueues[ track ];
    const UInt32 wanted = frameCapacity * queue.frameSize;
    while ( queue.data.GetDataSize() - queue.readOffset < wanted )
    {
        if ( m_consumesPictures && m_queuedFrames.size() >= m_maxQueuedFrames )
            break;
        if ( !DecodeOne( m_consumesPictures ) )
            break;
    }

    const UInt32 available = ( queue.data.GetDataSize() - queue.readOffset ) / queue.frameSize;
    const UInt32 frames = available < frameCapacity ? available : frameCapacity;
    if ( frames > 0 )
    {
        memcpy( destination, queue.data.GetConstBufferPtr( queue.readOffset ), (size_t) frames * queue.frameSize );
        queue.readOffset += frames * queue.frameSize;
    }

    // keep the buffer from growing: drop what was read
    const UInt32 remaining = queue.data.GetDataSize() - queue.readOffset;
    if ( 0 == remaining )
    {
        queue.data.SetDataSize( 0 );
        queue.readOffset = 0;
    }
    else
    if ( queue.readOffset > 65536 && queue.readOffset > remaining )
    {
        UInt8* bytes = static_cast< UInt8* >( queue.data.GetBufferPtr() );
        memmove( bytes, bytes + queue.readOffset, remaining );
        queue.data.SetDataSize( remaining );
        queue.readOffset = 0;
    }

    ended = 0 == remaining && ( m_source.IsNULL() || m_source->IsEndOfStream() );
    return frames;
}

/*-------------------------------------------------------------------------*/

void
CVideoDemuxer::SubscribeTrack( UInt32 track, bool subscribe )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( track >= m_audioQueues.size() )
        return;

    SAudioQueue& queue = m_audioQueues[ track ];
    if ( subscribe )
    {
        ++queue.subscribers;
    }
    else
    if ( queue.subscribers > 0 )
    {
        --queue.subscribers;
        if ( 0 == queue.subscribers )
        {
            queue.data.SetDataSize( 0 );
            queue.readOffset = 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

AUDIO::TAudioSourcePtr
CVideoDemuxer::CreateAudioTrackSource( const TVideoDemuxerPtr& demuxer, UInt32 track )
{GUCEF_TRACE;

    AUDIO::CAudioFormat format;
    if ( demuxer.IsNULL() || !demuxer->GetAudioTrackFormat( track, format ) )
        return AUDIO::TAudioSourcePtr();

    return AUDIO::TAudioSourcePtr( GUCEF_NEW CVideoAudioTrackSource( demuxer, track, format ) );
}

/*-------------------------------------------------------------------------*/

void
CVideoDemuxer::SetPictureConsumer( bool consumesPictures )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_consumesPictures = consumesPictures;
    if ( !consumesPictures )
    {
        for ( TFrameList::iterator i = m_queuedFrames.begin(); i != m_queuedFrames.end(); ++i )
            m_freeFrames.push_back( (*i) );
        m_queuedFrames.clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CVideoDemuxer::SetMaxQueuedFrames( UInt32 maxFrames )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxQueuedFrames = maxFrames;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoDemuxer::GetMaxQueuedFrames( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxQueuedFrames;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoDemuxer::GetQueuedFrameCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_queuedFrames.size();
}

/*-------------------------------------------------------------------------*/

bool
CVideoDemuxer::IsEndOfStream( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_queuedFrames.empty() && ( m_source.IsNULL() || m_source->IsEndOfStream() );
}

/*-------------------------------------------------------------------------*/

bool
CVideoDemuxer::HasFailed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_source.IsNULL() && m_source->HasFailed();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
