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

#ifndef GUCEF_VIDEO_CASYNCVIDEOTRANSCODE_H
#define GUCEF_VIDEO_CASYNCVIDEOTRANSCODE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_VIDEO_CVIDEOTRANSCODER_H
#include "gucefVIDEO_CVideoTranscoder.h"
#define GUCEF_VIDEO_CVIDEOTRANSCODER_H
#endif /* GUCEF_VIDEO_CVIDEOTRANSCODER_H ? */

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

/** What CVideoTranscoder::TranscodeAsync() asks a thread of a pool to do, and what came of it */
class GUCEF_HIDDEN CVideoTranscodeTaskData : public CORE::CICloneable
{
    public:

    TVideoSourcePtr source;
    TVideoSinkPtr sink;
    CString upscaler;
    UInt32 factor;
    UInt32 parallelUpscales;
    TVideoTranscodeStatusPtr status;    /**< shared with the caller */
    bool success;

    CVideoTranscodeTaskData( void );

    CVideoTranscodeTaskData( const CVideoTranscodeTaskData& src );

    virtual ~CVideoTranscodeTaskData();

    CVideoTranscodeTaskData& operator=( const CVideoTranscodeTaskData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The task that runs CVideoTranscoder::Transcode() for CVideoTranscoder::TranscodeAsync() */
class GUCEF_HIDDEN CAsyncVideoTranscode : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CAsyncVideoTranscode( void );

    virtual ~CAsyncVideoTranscode() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the outcome for CVideoTranscoder::GetAsyncResult() */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAsyncVideoTranscode( const CAsyncVideoTranscode& src );
    CAsyncVideoTranscode& operator=( const CAsyncVideoTranscode& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CAsyncVideoTranscode, MT::CMutex > TAsyncVideoTranscodeTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CASYNCVIDEOTRANSCODE_H ? */
