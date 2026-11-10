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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#include "gucefVIDEO_CAsyncVideoTranscode.h"
#include "gucefVIDEO_CVideoTranscoder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The sound of a frame as it was decoded, kept until its picture is written: the sound of a frame goes to the sink just before it */
struct SFrameSound
{
    UInt32 track;
    UInt32 frameCount;
    GUCEF::vector< UInt8 > bytes;
};

/*-------------------------------------------------------------------------*/

/** Collects the sound of a decoded frame for the tracks the sink has */
class CTranscodeAudioSink : public IVideoAudioSink
{
    public:

    CTranscodeAudioSink( const IVideoSource& source, UInt32 sinkTracks )
        : m_bytesPerFrame()
        , m_sound( GUCEF_NULL )
    {GUCEF_TRACE;

        for ( UInt32 t=0; t<source.GetAudioTrackCount() && t<sinkTracks; ++t )
        {
            AUDIO::CAudioFormat format;
            source.GetAudioTrackFormat( t, format );
            m_bytesPerFrame.push_back( format.GetBytesPerFrame() );
        }
    }

    /** Where the sound of the frame that is decoded next goes */
    void SetTarget( GUCEF::vector< SFrameSound >* sound )
    {GUCEF_TRACE;

        m_sound = sound;
    }

    virtual void OnVideoAudio( UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_sound || track >= m_bytesPerFrame.size() || GUCEF_NULL == frames || 0 == frameCount )
            return;
        SFrameSound piece;
        piece.track = track;
        piece.frameCount = frameCount;
        const UInt8* data = static_cast< const UInt8* >( frames );
        piece.bytes.assign( data, data + (size_t) frameCount * m_bytesPerFrame[ track ] );
        m_sound->push_back( piece );
    }

    private:

    GUCEF::vector< UInt32 > m_bytesPerFrame;
    GUCEF::vector< SFrameSound >* m_sound;
};

/*-------------------------------------------------------------------------*/

/** A frame on its way through the pipeline: decoded, its picture being upscaled, waiting to be encoded */
struct SPendingFrame
{
    UInt64 frameIndex;
    UInt64 timeInMicroSeconds;
    GUCEF::vector< SFrameSound > sound;
    CORE::CFutureResult upscale;                    /**< the upscale on the pool, when it was queued */
    IMAGE::TPixelMapPtr upscaled;                   /**< the upscaled picture, when it was made on this thread */
    CVideoFrame picture;                            /**< the picture as it is, without an upscaler */

    SPendingFrame( void ) : frameIndex( 0 ), timeInMicroSeconds( 0 ), sound(), upscale( CORE::TTaskStatus::TASKSTATUS_UNDEFINED ), upscaled(), picture() {}
};

/*-------------------------------------------------------------------------*/

/** Stops the upscale of a picture when the transcode is asked to stop */
class CTranscodeUpscaleProgress : public IMAGE::CIUpscaleProgress
{
    public:

    explicit CTranscodeUpscaleProgress( CORE::CTaskPtr task )
        : m_task( task )
    {GUCEF_TRACE;

    }

    virtual bool OnUpscaleProgress( Float32 fraction ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_task.IsNULL() || !m_task->IsCancellationRequested();
    }

    private:

    CORE::CTaskPtr m_task;
};

/*-------------------------------------------------------------------------*/

/** Keeps the status of an asynchronous transcode and stops it when its task is asked to */
class CTaskTranscodeProgress : public CIVideoTranscodeProgress
{
    public:

    CTaskTranscodeProgress( CORE::CTaskPtr task, const TVideoTranscodeStatusPtr& status )
        : m_task( task )
        , m_status( status )
    {GUCEF_TRACE;

    }

    virtual bool OnTranscodeProgress( UInt64 framesDone, UInt64 framesTotal ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_status.IsNULL() )
            m_status->SetProgress( framesDone, framesTotal );
        return !m_task->IsCancellationRequested();
    }

    private:

    CORE::CTaskPtr m_task;
    TVideoTranscodeStatusPtr m_status;
};

/*-------------------------------------------------------------------------*/

bool
WriteSound( IVideoSink& sink, const GUCEF::vector< SFrameSound >& sound )
{GUCEF_TRACE;

    for ( size_t i=0; i<sound.size(); ++i )
        if ( !sink.WriteAudio( sound[ i ].track, &sound[ i ].bytes[ 0 ], sound[ i ].frameCount ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Stops the upscales that are still on the pool and waits for them, so nothing works on for a transcode that ended */
void
AbandonFrames( GUCEF::vector< SPendingFrame* >& pending )
{GUCEF_TRACE;

    for ( size_t i=0; i<pending.size(); ++i )
        if ( pending[ i ]->upscale.HasAFuture() && !pending[ i ]->upscale.IsReady() )
            pending[ i ]->upscale.RequestCancellation();
    for ( size_t i=0; i<pending.size(); ++i )
    {
        if ( pending[ i ]->upscale.HasAFuture() )
            pending[ i ]->upscale.Await();
        GUCEF_DELETE pending[ i ];
    }
    pending.clear();
}

/*-------------------------------------------------------------------------*/

bool
TranscodeFrames( const TVideoSourcePtr& sourcePtr, const TVideoSinkPtr& sinkPtr, const CString& upscalerName, UInt32 factor,
                 UInt32 parallelUpscales, CIVideoTranscodeProgress* progress, CORE::CTaskPtr task )
{GUCEF_TRACE;

    TVideoSourcePtr source( sourcePtr );
    TVideoSinkPtr sink( sinkPtr );
    if ( source.IsNULL() || sink.IsNULL() || 0 == factor )
        return false;

    IMAGE::CImageUpscaler* upscaler = GUCEF_NULL;
    if ( !upscalerName.IsNULLOrEmpty() )
    {
        upscaler = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( upscalerName );
        if ( GUCEF_NULL == upscaler || factor < 2 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: There is no image upscaler \"" << upscalerName << "\" for a factor of " << factor );
            return false;
        }
    }
    else
        factor = 1;

    const CVideoFormat& in = source->GetFormat();
    const CVideoFormat& out = sink->GetFormat();
    if ( out.GetWidth() != in.GetWidth() * factor || out.GetHeight() != in.GetHeight() * factor )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: The sink of " << out.GetWidth() << "x" << out.GetHeight() << " does not fit a source of " <<
                         in.GetWidth() << "x" << in.GetHeight() << " upscaled " << factor << " times" );
        return false;
    }

    // the pictures are upscaled on the threads of a pool, several at once, while this thread decodes the next ones and encodes
    // the finished ones in their order. Where the pool cannot take them (no threads) they are upscaled here, one at a time
    UInt32 window = 1;
    if ( GUCEF_NULL != upscaler )
    {
        if ( 0 == parallelUpscales )
            parallelUpscales = CORE::GetLogicalCPUCount();
        if ( 0 == parallelUpscales )
            parallelUpscales = 1;
        window = parallelUpscales + 1;
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CVideoTranscoder::UpscaleThreadPoolName );
        if ( !pool.IsNULL() && pool->GetDesiredMinNrOfWorkerThreads() < parallelUpscales )
            pool->SetDesiredMinNrOfWorkerThreads( parallelUpscales );
    }

    const CVideoFormat upscaledFormat( out.GetWidth(), out.GetHeight(), IMAGE::PSF_RGBA, in.GetFrameRateNumerator(), in.GetFrameRateDenominator(), in.GetFrameCount() );
    CVideoFrame frame;
    CVideoFrame upscaledFrame;
    CTranscodeAudioSink audio( *source.GetPointerAlways(), sink->GetAudioTrackCount() );
    CTranscodeUpscaleProgress upscaleProgress( task );
    GUCEF::vector< SPendingFrame* > pending;
    bool decodedAll = false;
    UInt64 done = 0;

    // where the time goes, for the log at the end: decoding (with queueing the upscales), waiting for upscales, encoding
    const UInt64 started = MT::PrecisionTickCount();
    UInt64 decodeTicks = 0, waitTicks = 0, encodeTicks = 0;
    while ( true )
    {
        // decode until the window is full
        UInt64 ticks = MT::PrecisionTickCount();
        while ( !decodedAll && pending.size() < window )
        {
            SPendingFrame* next = GUCEF_NEW SPendingFrame();
            audio.SetTarget( &next->sound );
            if ( !source->DecodeNextFrame( frame, &audio ) )
            {
                GUCEF_DELETE next;
                decodedAll = true;
                break;
            }
            next->frameIndex = frame.GetFrameIndex();
            next->timeInMicroSeconds = frame.GetTimeInMicroSeconds();
            pending.push_back( next );
            if ( GUCEF_NULL == upscaler )
            {
                next->picture = frame;
                continue;
            }

            IMAGE::TPixelMapPtr rgba;
            if ( !frame.ConvertTo32Bit( IMAGE::PSF_RGBA, rgba ) || rgba.IsNULL() )
            {
                AbandonFrames( pending );
                return false;
            }
            // its own copy: the frame is decoded into again while the pool works on it
            IMAGE::TPixelMapPtr copy( GUCEF_NEW IMAGE::CPixelMap( *rgba ) );
            next->upscale = upscaler->UpscaleAsync( copy, factor, CVideoTranscoder::UpscaleThreadPoolName );
            if ( next->upscale.HasNoFuture() && ( !upscaler->Upscale( *copy, factor, next->upscaled, &upscaleProgress ) || next->upscaled.IsNULL() ) )
            {
                if ( task.IsNULL() || !task->IsCancellationRequested() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: The upscale of frame " << next->frameIndex << " failed" );
                AbandonFrames( pending );
                return false;
            }
        }
        audio.SetTarget( GUCEF_NULL );
        decodeTicks += MT::PrecisionTickCount() - ticks;
        if ( pending.empty() )
            break;

        // the oldest frame: its sound, then its picture
        SPendingFrame* current = pending.front();
        ticks = MT::PrecisionTickCount();
        if ( current->upscale.HasAFuture() )
        {
            current->upscale.Await();
            if ( !IMAGE::CImageUpscaler::GetAsyncResult( current->upscale, current->upscaled ) )
            {
                if ( task.IsNULL() || !task->IsCancellationRequested() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: The upscale of frame " << current->frameIndex << " failed" );
                AbandonFrames( pending );
                return false;
            }
        }
        const UInt64 waited = MT::PrecisionTickCount();
        waitTicks += waited - ticks;
        bool written = WriteSound( *sink.GetPointerAlways(), current->sound );
        if ( written && GUCEF_NULL != upscaler )
        {
            const IMAGE::TPixelMapPtr& upscaled = current->upscaled;
            if ( upscaled.IsNULL() || IMAGE::PSF_RGBA != upscaled->GetPixelStorageFormat() || upscaled->GetWidthInPixels() != out.GetWidth() ||
                 upscaled->GetHeightInPixels() != out.GetHeight() || !upscaledFrame.Prepare( upscaledFormat ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: The upscaler \"" << upscalerName << "\" gave a picture of another size or format" );
                AbandonFrames( pending );
                return false;
            }
            const UInt32 rowBytes = out.GetWidth() * 4;
            const UInt8* from = static_cast< const UInt8* >( upscaled->GetDataPtr() );
            UInt8* to = static_cast< UInt8* >( upscaledFrame.GetPicture().GetDataPtr() );
            for ( UInt32 y=0; y<out.GetHeight(); ++y )
                memcpy( to + (size_t) y * upscaledFrame.GetPitch(), from + (size_t) y * rowBytes, rowBytes );
            upscaledFrame.SetFrameIndex( current->frameIndex );
            upscaledFrame.SetTimeInMicroSeconds( current->timeInMicroSeconds );
            written = sink->WriteFrame( upscaledFrame );
        }
        else
        if ( written )
        {
            written = sink->WriteFrame( current->picture );
        }
        encodeTicks += MT::PrecisionTickCount() - waited;
        pending.erase( pending.begin() );
        GUCEF_DELETE current;
        if ( !written )
        {
            AbandonFrames( pending );
            return false;
        }

        ++done;
        if ( GUCEF_NULL != progress && !progress->OnTranscodeProgress( done, in.GetFrameCount() ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: Stopped after " << done << " frames" );
            AbandonFrames( pending );
            return false;
        }
    }
    if ( source->HasFailed() )
        return false;
    UInt64 ticks = MT::PrecisionTickCount();
    const bool finished = sink->Finish();
    encodeTicks += MT::PrecisionTickCount() - ticks;
    const UInt64 resolution = MT::PrecisionTimerResolution() > 0 ? MT::PrecisionTimerResolution() / 1000 : 1;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VideoTranscoder: " << done << " frames of " << out.GetWidth() << "x" << out.GetHeight() << " in " <<
                      ( MT::PrecisionTickCount() - started ) / resolution << " ms: decoding " << decodeTicks / resolution << " ms, waiting for upscales " <<
                      waitTicks / resolution << " ms, encoding " << encodeTicks / resolution << " ms" );
    return finished;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIVideoTranscodeProgress::~CIVideoTranscodeProgress()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoTranscodeStatus::CVideoTranscodeStatus( void )
    : m_lock()
    , m_framesDone( 0 )
    , m_framesTotal( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN )
    , m_finished( false )
    , m_successful( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CVideoTranscodeStatus::GetFramesDone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_framesDone;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoTranscodeStatus::GetFramesTotal( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_framesTotal;
}

/*-------------------------------------------------------------------------*/

bool
CVideoTranscodeStatus::IsFinished( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_finished;
}

/*-------------------------------------------------------------------------*/

bool
CVideoTranscodeStatus::IsSuccessful( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_successful;
}

/*-------------------------------------------------------------------------*/

void
CVideoTranscodeStatus::SetProgress( UInt64 framesDone, UInt64 framesTotal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_framesDone = framesDone;
    m_framesTotal = framesTotal;
}

/*-------------------------------------------------------------------------*/

void
CVideoTranscodeStatus::SetFinished( bool successful )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_finished = true;
    m_successful = successful;
}

/*-------------------------------------------------------------------------*/

const CString CVideoTranscoder::UpscaleThreadPoolName = "gucefVIDEO::TranscodeUpscale";

/*-------------------------------------------------------------------------*/

bool
CVideoTranscoder::Transcode( const TVideoSourcePtr& source       ,
                             const TVideoSinkPtr& sink           ,
                             const CString& upscaler             ,
                             UInt32 factor                       ,
                             CIVideoTranscodeProgress* progress  ,
                             UInt32 parallelUpscales             )
{GUCEF_TRACE;

    return TranscodeFrames( source, sink, upscaler, factor, parallelUpscales, progress, CORE::CTaskPtr() );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVideoTranscoder::TranscodeAsync( const TVideoSourcePtr& source       ,
                                  const TVideoSinkPtr& sink           ,
                                  const CString& upscaler             ,
                                  UInt32 factor                       ,
                                  TVideoTranscodeStatusPtr& status    ,
                                  const CString& threadPoolName       ,
                                  UInt32 parallelUpscales             )
{GUCEF_TRACE;

    status = TVideoTranscodeStatusPtr( GUCEF_NEW CVideoTranscodeStatus() );
    try
    {
        CVideoTranscodeTaskData taskData;
        taskData.source = source;
        taskData.sink = sink;
        taskData.upscaler = upscaler;
        taskData.factor = factor;
        taskData.parallelUpscales = parallelUpscales;
        taskData.status = status;

        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : threadPoolName );
        if ( !threadPool.IsNULL() )
            return threadPool->QueueTask( CAsyncVideoTranscode::TaskType, &taskData );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VideoTranscoder:TranscodeAsync: Failed to queue the task: " << e.what() );
    }
    status->SetFinished( false );
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVideoTranscoder::GetAsyncResult( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return false;
    const CVideoTranscodeTaskData* data = static_cast< const CVideoTranscodeTaskData* >( task->GetTaskData() );
    return GUCEF_NULL != data && data->success;
}

/*-------------------------------------------------------------------------*/

const CORE::CString CAsyncVideoTranscode::TaskType = "AsyncVideoTranscode";

/*-------------------------------------------------------------------------*/

CVideoTranscodeTaskData::CVideoTranscodeTaskData( void )
    : CORE::CICloneable()
    , source()
    , sink()
    , upscaler()
    , factor( 1 )
    , parallelUpscales( 0 )
    , status()
    , success( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoTranscodeTaskData::CVideoTranscodeTaskData( const CVideoTranscodeTaskData& src )
    : CORE::CICloneable( src )
    , source( src.source )
    , sink( src.sink )
    , upscaler( src.upscaler )
    , factor( src.factor )
    , parallelUpscales( src.parallelUpscales )
    , status( src.status )
    , success( src.success )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoTranscodeTaskData::~CVideoTranscodeTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoTranscodeTaskData&
CVideoTranscodeTaskData::operator=( const CVideoTranscodeTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        source = src.source;
        sink = src.sink;
        upscaler = src.upscaler;
        factor = src.factor;
        parallelUpscales = src.parallelUpscales;
        status = src.status;
        success = src.success;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVideoTranscodeTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CVideoTranscodeTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CAsyncVideoTranscode::CAsyncVideoTranscode( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncVideoTranscode::~CAsyncVideoTranscode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAsyncVideoTranscode::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncVideoTranscode::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CVideoTranscodeTaskData* data = static_cast< CVideoTranscodeTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVideoTranscode: No task data to work on" );
        return true;
    }

    CTaskTranscodeProgress progress( task, data->status );
    data->success = TranscodeFrames( data->source, data->sink, data->upscaler, data->factor, data->parallelUpscales, &progress, task );

    // the source and the sink are done with: what they hold goes now, not when the caller lets go of the future
    data->source.Unlink();
    data->sink.Unlink();
    if ( !data->status.IsNULL() )
        data->status->SetFinished( data->success );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncVideoTranscode::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return;
    CVideoTranscodeTaskData* data = static_cast< CVideoTranscodeTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data && !data->status.IsNULL() && !data->status->IsFinished() )
        data->status->SetFinished( false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
