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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILETRANSFERTASKDATA_H
#include "gucefASSETSTORE_CRemoteFileTransferTaskData.h"
#define GUCEF_ASSETSTORE_CREMOTEFILETRANSFERTASKDATA_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILETRANSFERTASKDATA_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#include "gucefASSETSTORE_CAssetServiceCall.h"
#define GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICECALL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICERESULT_H
#include "gucefASSETSTORE_CAssetServiceResult.h"
#define GUCEF_ASSETSTORE_CASSETSERVICERESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICERESULT_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#include "gucefASSETSTORE_CRemoteFileTransferTaskConsumer.h"

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

const CORE::CString CRemoteFileTransferTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CRemoteFileTransferTaskConsumer";

/*-------------------------------------------------------------------------*/

CRemoteFileTransferTaskConsumer::CRemoteFileTransferTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteFileTransferTaskConsumer::~CRemoteFileTransferTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteFileTransferTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteFileTransferTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CRemoteFileTransferTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileTransferTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CRemoteFileTransferTaskData* data = const_cast< CRemoteFileTransferTaskData* >( CORE::CICloneable::CloneableAs< CRemoteFileTransferTaskData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->system || data->transfer.IsNULL() )
        return true;
    CIRemoteFileSource* system = data->system;
    TRemoteFileTransferPtr transfer = data->transfer;
    data->system = GUCEF_NULL;
    CAssetServiceCall call( "Files/Transfer" );
    try
    {
        transfer->Run( *system );
    }
    catch ( ... )
    {
        CIAssetStoreService::CompleteCall( data->source, call, ASSETSTORE_STATUS_FAILED, GUCEF_NULL, "an exception ended the transfer" );
        system->TransferEnded( transfer );
        return true;
    }
    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    result->status = transfer->GetStatus();
    result->message = transfer->GetMessage();
    CIAssetStoreService::CompleteCall( data->source, call, transfer->GetStatus(), result, transfer->GetMessage() );
    system->TransferEnded( transfer );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransferTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        CRemoteFileTransferTaskData* data = const_cast< CRemoteFileTransferTaskData* >( CORE::CICloneable::CloneableAs< CRemoteFileTransferTaskData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->system )
        {
            // the task ended before it ran
            CIRemoteFileSource* system = data->system;
            data->system = GUCEF_NULL;
            data->transfer->Cancel();
            data->source.Cancel();
            system->TransferEnded( data->transfer );
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
