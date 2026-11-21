/*
 *  AssetConverterLib: the task that converts one asset on a thread pool
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#include "AssetConverterLib_CAssetConversionJob.h"
#define ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#include "AssetConverterLib_CAssetConversionTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetConversionTaskConsumer::TaskType = "AssetConversion";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetConversionTaskConsumer::CAssetConversionTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionTaskConsumer::~CAssetConversionTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetConversionTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CAssetConversionJob* job = CORE::CICloneable::CloneableAs< CAssetConversionJob >( task->GetTaskData() );
    if ( GUCEF_NULL == job )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetConversionTaskConsumer: No job to work on" );
        return true;
    }
    if ( task->IsCancellationRequested() )
        return true;
    job->report.SetCancellationSource( task.GetPointerAlways() );
    CAssetPipeline::ConvertAsset( *job );
    job->report.SetCancellationSource( GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetConversionTaskConsumer::OnTaskEnded( CORE::CTaskPtr /* task */, bool /* wasForced */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
