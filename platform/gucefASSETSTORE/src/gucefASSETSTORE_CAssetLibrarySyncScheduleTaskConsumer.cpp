/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKDATA_H
#include "gucefASSETSTORE_CAssetLibrarySyncScheduleTaskData.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKDATA_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncScheduleTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetLibrarySyncScheduleTaskConsumer::TaskType = "GUCEF::ASSETSTORE::AssetLibrarySyncScheduledRun";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetLibrarySyncScheduleTaskConsumer::CAssetLibrarySyncScheduleTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncScheduleTaskConsumer::~CAssetLibrarySyncScheduleTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncScheduleTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncScheduleTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // the run is the start of the sync; a run that could not start it fails at its start, with the reason
    if ( !CORE::CTaskConsumer::OnTaskStart( task ) )
        return false;
    const CAssetLibrarySyncScheduleTaskData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CAssetLibrarySyncScheduleTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        if ( !task.IsNULL() )
            task->SetTaskStatusExtraInfo( "failed: the run has no store" );
        return false;
    }
    CORE::CString jobId;
    CORE::CString message;
    const TAssetStoreStatus status = CLocalAssetStoreService::StartScheduledSync( data->serviceKey, data->storeId, jobId, message );
    if ( ASSETSTORE_STATUS_OK != status && ASSETSTORE_STATUS_ALREADY_EXISTS != status )
    {
        task->SetTaskStatusExtraInfo( "failed: " + message );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncScheduleTaskConsumer: The scheduled sync of store " << data->storeId << " was not started: " << message );
        return false;
    }
    task->SetTaskStatusExtraInfo( ( ASSETSTORE_STATUS_OK == status ? "started " : "runs already: " ) + jobId );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncScheduleTaskConsumer: The scheduled sync of store " << data->storeId << ( ASSETSTORE_STATUS_OK == status ? " is job " : " runs already as job " ) << jobId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncScheduleTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncScheduleTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncScheduleTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncScheduleTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
