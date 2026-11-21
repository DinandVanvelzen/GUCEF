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

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#include "gucefASSETSTORE_CIAssetStoreProbe.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H ? */

#include "gucefASSETSTORE_CAssetStoreProviderOperationTaskConsumer.h"

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

const CORE::CString CAssetStoreProviderOperationTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CAssetStoreProviderOperationTaskConsumer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreProviderOperationTaskConsumer::CAssetStoreProviderOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreProviderOperationTaskConsumer::~CAssetStoreProviderOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreProviderOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreProviderOperationTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreProviderOperationTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreProviderOperationTaskConsumer::FailSource( CORE::CFutureResultSource& source, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    if ( ASSETSTORE_STATUS_CANCELLED == status )
    {
        source.Cancel();
        return;
    }
    CORE::CString info = AssetStoreStatusToString( status );
    if ( !message.IsNULLOrEmpty() )
        info += ": " + message;
    source.Fail( CORE::TASKSTATUS_EXCEPTION_DURING_RUN, info );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreProviderOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CAssetStoreProviderOperationData* data = const_cast< CAssetStoreProviderOperationData* >( CORE::CICloneable::CloneableAs< CAssetStoreProviderOperationData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->provider )
        return true;

    CIAssetStoreProvider* provider = data->provider;
    data->provider = GUCEF_NULL;     // the provider is told once that the operation ended
    try
    {
        switch ( data->operation )
        {
            case CAssetStoreProviderOperationData::OPERATION_QUERY_LIBRARY:
            {
                CAssetStoreLibraryPage page;
                const TAssetStoreStatus status = provider->QueryLibrary( data->query, page );
                if ( ASSETSTORE_STATUS_OK == status )
                    data->source.Complete( page.Clone(), true );
                else
                    FailSource( data->source, status, CORE::CString::Empty );
                break;
            }
            case CAssetStoreProviderOperationData::OPERATION_GET_ITEM:
            {
                CAssetStoreItem item;
                const TAssetStoreStatus status = provider->GetItem( data->itemId, item );
                if ( ASSETSTORE_STATUS_OK == status )
                    data->source.Complete( item.Clone(), true );
                else
                    FailSource( data->source, status, CORE::CString::Empty );
                break;
            }
            case CAssetStoreProviderOperationData::OPERATION_PLAN_DOWNLOAD:
            {
                CAssetStoreDownloadPlan plan;
                const TAssetStoreStatus status = provider->PlanDownload( data->itemId, data->formatId, data->fileIds, plan );
                if ( ASSETSTORE_STATUS_OK == status )
                    data->source.Complete( plan.Clone(), true );
                else
                    FailSource( data->source, status, CORE::CString::Empty );
                break;
            }
            case CAssetStoreProviderOperationData::OPERATION_PROBE:
            {
                CIAssetStoreProbe* probe = provider->GetProbe();
                if ( GUCEF_NULL == probe )
                {
                    FailSource( data->source, ASSETSTORE_STATUS_UNSUPPORTED, "the store has no probe" );
                    break;
                }
                CAssetStoreProbeResult result;
                if ( probe->Run( result ) || !result.checks.empty() )
                    data->source.Complete( result.Clone(), true );
                else
                    FailSource( data->source, ASSETSTORE_STATUS_FAILED, result.summary );
                break;
            }
            default:
            {
                FailSource( data->source, ASSETSTORE_STATUS_INVALID_ARGUMENT, "unknown operation" );
                break;
            }
        }
    }
    catch ( ... )
    {
        FailSource( data->source, ASSETSTORE_STATUS_FAILED, "the provider threw an exception" );
    }
    provider->OperationEnded();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreProviderOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // a task that ended without running its operation still lets go of the provider and of its waiter
    if ( !task.IsNULL() )
    {
        CAssetStoreProviderOperationData* data = const_cast< CAssetStoreProviderOperationData* >( CORE::CICloneable::CloneableAs< CAssetStoreProviderOperationData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->provider )
        {
            CIAssetStoreProvider* provider = data->provider;
            data->provider = GUCEF_NULL;
            data->source.Cancel();
            provider->OperationEnded();
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
