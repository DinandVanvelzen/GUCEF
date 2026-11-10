/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_IMAGE_CASYNCIMAGEUPSCALE_H
#include "gucefIMAGE_CAsyncImageUpscale.h"
#define GUCEF_IMAGE_CASYNCIMAGEUPSCALE_H
#endif /* GUCEF_IMAGE_CASYNCIMAGEUPSCALE_H ? */

#include "gucefIMAGE_CImageUpscaler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIUpscaleProgress::~CIUpscaleProgress()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageUpscaler::CImageUpscaler( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageUpscaler::~CImageUpscaler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CImageUpscaler::GetFactorOfAtLeast( UInt32 wanted ) const
{GUCEF_TRACE;

    TFactorVector factors = GetFactors();
    if ( factors.empty() )
        return 0;
    for ( UInt32 i=0; i<(UInt32)factors.size(); ++i )
        if ( factors[ i ] >= wanted )
            return factors[ i ];
    return factors.back();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CImageUpscaler::UpscaleAsync( const TPixelMapPtr& source, UInt32 factor, const CORE::CString& threadPoolName ) const
{GUCEF_TRACE;

    try
    {
        CImageUpscaleTaskData taskData;
        taskData.upscalerName = GetName();
        taskData.factor = factor;
        taskData.source = source;

        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : threadPoolName );
        if ( !threadPool.IsNULL() )
            return threadPool->QueueTask( CAsyncImageUpscale::TaskType, &taskData );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ImageUpscaler:UpscaleAsync: Failed to queue the task: " << e.what() );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CImageUpscaler::GetAsyncResult( const CORE::CFutureResult& future, TPixelMapPtr& result )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return false;
    const CImageUpscaleTaskData* data = static_cast< const CImageUpscaleTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data || !data->success || data->result.IsNULL() )
        return false;
    result = data->result;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
