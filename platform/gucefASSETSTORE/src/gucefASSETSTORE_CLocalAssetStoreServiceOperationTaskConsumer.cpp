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

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CLocalAssetStoreServiceOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLocalAssetStoreServiceOperationTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CLocalAssetStoreServiceOperationTaskConsumer";

/*-------------------------------------------------------------------------*/

CLocalAssetStoreServiceOperationTaskConsumer::CLocalAssetStoreServiceOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalAssetStoreServiceOperationTaskConsumer::~CLocalAssetStoreServiceOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalAssetStoreServiceOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLocalAssetStoreServiceOperationTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CLocalAssetStoreServiceOperationTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreServiceOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CLocalAssetStoreServiceOperationData* data = const_cast< CLocalAssetStoreServiceOperationData* >( CORE::CICloneable::CloneableAs< CLocalAssetStoreServiceOperationData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->service )
        return true;
    CLocalAssetStoreService* service = data->service;
    data->service = GUCEF_NULL;
    try
    {
        if ( !data->pipelineJobKey.IsNULLOrEmpty() )
        {
            // the job ends its own future
            service->RunPipelineJob( data->pipelineJobKey );
        }
        else
        if ( data->source.IsCancellationRequested() )
        {
            data->source.Cancel();
        }
        else
        {
            CORE::CICloneable* result = GUCEF_NULL;
            CORE::CString message;
            const TAssetStoreStatus status = service->Dispatch( data->call, result, message );
            CIAssetStoreService::CompleteCall( data->source, data->call, status, result, message );
        }
    }
    catch ( ... )
    {
        CIAssetStoreService::CompleteCall( data->source, data->call, ASSETSTORE_STATUS_FAILED, GUCEF_NULL, "an exception ended the call" );
    }
    service->CallEnded();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreServiceOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        CLocalAssetStoreServiceOperationData* data = const_cast< CLocalAssetStoreServiceOperationData* >( CORE::CICloneable::CloneableAs< CLocalAssetStoreServiceOperationData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->service )
        {
            // the task ended before it ran
            CLocalAssetStoreService* service = data->service;
            data->service = GUCEF_NULL;
            data->source.Cancel();
            service->CallEnded();
        }
    }
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
