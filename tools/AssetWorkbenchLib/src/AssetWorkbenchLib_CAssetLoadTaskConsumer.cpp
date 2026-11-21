/*
 *  AssetWorkbenchLib: the pool task that prepares an asset for an open
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

#include "AssetWorkbenchLib_CAssetLoadTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {


const CORE::CString CAssetLoadTaskConsumer::TaskType = "AssetWorkbenchLoad";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetLoadTaskConsumer::CAssetLoadTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLoadTaskConsumer::~CAssetLoadTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLoadTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLoadTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CAssetLoadTaskData* data = static_cast< CAssetLoadTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->job.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetLoadTaskConsumer: No job to prepare" );
        return true;
    }
    CAssetLoadControl control( data->activity, data->activityId, task.GetPointerAlways() );
    try
    {
        data->prepared = data->job->Prepare( control );
    }
    catch ( const std::exception& e )
    {
        data->prepared = false;
        data->problem = CORE::CString( "The asset could not be prepared: " ) + e.what();
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetLoadTaskConsumer: " << data->problem );
    }
    catch ( ... )
    {
        data->prepared = false;
        data->problem = "The asset could not be prepared";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetLoadTaskConsumer: " << data->problem );
    }
    data->cancelled = control.IsCancelled();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetLoadTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
