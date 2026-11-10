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

#ifndef GUCEF_VIDEO_CVIDEOTRANSCODER_H
#define GUCEF_VIDEO_CVIDEOTRANSCODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSOURCE_H
#include "gucefVIDEO_IVideoSource.h"
#define GUCEF_VIDEO_IVIDEOSOURCE_H
#endif /* GUCEF_VIDEO_IVIDEOSOURCE_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSINK_H
#include "gucefVIDEO_IVideoSink.h"
#define GUCEF_VIDEO_IVIDEOSINK_H
#endif /* GUCEF_VIDEO_IVIDEOSINK_H ? */

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

/** Hears how far a transcode is */
class GUCEF_VIDEO_PUBLIC_CPP CIVideoTranscodeProgress
{
    public:

    virtual ~CIVideoTranscodeProgress();

    /**
     *  After each frame. 'framesTotal' is GUCEF_VIDEO_FRAMECOUNT_UNKNOWN when the source does not tell.
     *  Returning false stops the transcode
     */
    virtual bool OnTranscodeProgress( UInt64 framesDone, UInt64 framesTotal ) = 0;
};

/*-------------------------------------------------------------------------*/

/** How an asynchronous transcode is doing, shared between the task and the caller */
class GUCEF_VIDEO_PUBLIC_CPP CVideoTranscodeStatus
{
    public:

    typedef CORE::CTSharedPtr< CVideoTranscodeStatus, MT::CMutex > TVideoTranscodeStatusPtr;

    CVideoTranscodeStatus( void );

    UInt64 GetFramesDone( void ) const;

    /** GUCEF_VIDEO_FRAMECOUNT_UNKNOWN when the source does not tell */
    UInt64 GetFramesTotal( void ) const;

    /**
     *  True once the transcode ran to its end, whatever the outcome. A task that is cancelled before it starts
     *  never runs and leaves this false: the future tells when it is over
     */
    bool IsFinished( void ) const;

    /** True when it ended with the output complete */
    bool IsSuccessful( void ) const;

    void SetProgress( UInt64 framesDone, UInt64 framesTotal );

    void SetFinished( bool successful );

    private:

    MT::CMutex m_lock;
    UInt64 m_framesDone;
    UInt64 m_framesTotal;
    bool m_finished;
    bool m_successful;
};

typedef CVideoTranscodeStatus::TVideoTranscodeStatusPtr TVideoTranscodeStatusPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Takes a video from a source to a sink: every picture, optionally upscaled by an upscaler of the gucefIMAGE
 *  upscaler registry, with the sound of the audio tracks the sink has (tracks of the source beyond those are
 *  left out). The sink is finished at the end. A failed or stopped transcode leaves the output incomplete.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoTranscoder
{
    public:

    /** The thread pool the pictures are upscaled on, several at once */
    static const CString UpscaleThreadPoolName;

    /**
     *  Runs on the calling thread. 'upscaler' empty for none, else the sink is 'factor' times as large as the source.
     *  'progress' may be NULL. The pictures are upscaled on the threads of UpscaleThreadPoolName, 'parallelUpscales' at
     *  once (0 for one per logical processor; an upscaler on the GPU gains little from more than 2) while this thread
     *  decodes the next ones and has the finished ones encoded in their order; where the pool has no threads they are
     *  upscaled on this thread
     */
    static bool Transcode( const TVideoSourcePtr& source         ,
                           const TVideoSinkPtr& sink             ,
                           const CString& upscaler               ,
                           UInt32 factor                         ,
                           CIVideoTranscodeProgress* progress    ,
                           UInt32 parallelUpscales = 0           );

    /**
     *  The same as a task on a thread of the pool. The source and the sink belong to the task until it ends;
     *  'status' is made here and tells how far it is; the future is ready when it is over, also when it was
     *  cancelled before it started. RequestCancellation() on the future stops it.
     */
    static CORE::CFutureResult TranscodeAsync( const TVideoSourcePtr& source                ,
                                               const TVideoSinkPtr& sink                    ,
                                               const CString& upscaler                      ,
                                               UInt32 factor                                ,
                                               TVideoTranscodeStatusPtr& status             ,
                                               const CString& threadPoolName = CString::Empty ,
                                               UInt32 parallelUpscales = 0                  );

    /** Whether a TranscodeAsync() whose future is ready completed its output */
    static bool GetAsyncResult( const CORE::CFutureResult& future );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOTRANSCODER_H ? */
