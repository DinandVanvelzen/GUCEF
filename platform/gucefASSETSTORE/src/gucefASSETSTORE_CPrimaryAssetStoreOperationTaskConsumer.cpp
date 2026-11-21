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
#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetStoreProviderOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#include "gucefASSETSTORE_CPrimaryAssetStoreOperationTaskConsumer.h"

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

const CORE::CString CPrimaryAssetStoreOperationTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CPrimaryAssetStoreOperationTaskConsumer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPrimaryAssetStoreOperationTaskConsumer::CPrimaryAssetStoreOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPrimaryAssetStoreOperationTaskConsumer::~CPrimaryAssetStoreOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStoreOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPrimaryAssetStoreOperationTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CPrimaryAssetStoreOperationTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStoreOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CPrimaryAssetStoreOperationData* data = const_cast< CPrimaryAssetStoreOperationData* >( CORE::CICloneable::CloneableAs< CPrimaryAssetStoreOperationData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->store )
        return true;
    CPrimaryAssetStore* store = data->store;
    data->store = GUCEF_NULL;
    try
    {
        switch ( data->operation )
        {
            case CPrimaryAssetStoreOperationData::OPERATION_INGEST:
            case CPrimaryAssetStoreOperationData::OPERATION_RESUME_INGEST:
            {
                CAssetIngestResult result;
                if ( CPrimaryAssetStoreOperationData::OPERATION_INGEST == data->operation )
                    store->RunIngest( data->request, result, &data->source );
                else
                    store->RunResumeIngest( data->ingestId, result, &data->source );
                // a duplicate is an outcome, not a failure: the result tells
                if ( ASSETSTORE_STATUS_OK == result.status || ASSETSTORE_STATUS_DUPLICATE == result.status )
                    data->source.Complete( result.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, result.status, result.message );
                break;
            }
            case CPrimaryAssetStoreOperationData::OPERATION_CONVERT_PACK:
            case CPrimaryAssetStoreOperationData::OPERATION_CONVERT_PENDING:
            {
                CAssetConversionOutcome outcome;
                if ( CPrimaryAssetStoreOperationData::OPERATION_CONVERT_PACK == data->operation )
                    store->RunConvertPack( data->packId, outcome, &data->source );
                else
                    store->RunConvertPending( outcome, &data->source );

                // a refusal, and a conversion that ended with errors, are outcomes and not failures of the operation: the outcome tells
                if ( ASSETSTORE_STATUS_OK == outcome.status || ASSETSTORE_STATUS_REFUSED == outcome.status || ASSETSTORE_STATUS_FAILED == outcome.status )
                    data->source.Complete( outcome.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, outcome.status, outcome.message );
                break;
            }
            case CPrimaryAssetStoreOperationData::OPERATION_SEARCH:
            {
                CAssetSearchResultPage page;
                const TAssetStoreStatus status = store->Search( data->query, page );
                if ( ASSETSTORE_STATUS_OK == status )
                    data->source.Complete( page.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, status, store->GetLastErrorMessage() );
                break;
            }
            case CPrimaryAssetStoreOperationData::OPERATION_MIGRATE_LAYOUT:
            {
                CAssetLayoutMigrationReport report;
                const TAssetStoreStatus status = store->MigrateStorageLayout( data->locationId, data->dryRun, report );
                if ( ASSETSTORE_STATUS_OK == status )
                    data->source.Complete( report.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, status, store->GetLastErrorMessage() );
                break;
            }
            case CPrimaryAssetStoreOperationData::OPERATION_INDEX_PACK:
            {
                CAssetPackIndexOutcome outcome;
                store->RunIndexPack( data->packId, data->indexOptions, outcome, &data->source );
                if ( ASSETSTORE_STATUS_OK == outcome.status || ASSETSTORE_STATUS_UNSUPPORTED == outcome.status )
                    data->source.Complete( outcome.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, outcome.status, outcome.message );
                break;
            }
            case CPrimaryAssetStoreOperationData::OPERATION_INDEX_MISSING:
            case CPrimaryAssetStoreOperationData::OPERATION_REINDEX_FROM_STORAGE:
            {
                CAssetPackIndexReport report;
                if ( CPrimaryAssetStoreOperationData::OPERATION_INDEX_MISSING == data->operation )
                    store->RunIndexPacksWithoutIndex( data->indexOptions, report, &data->source );
                else
                    store->RunReindexFromStorage( data->buildMissing, data->indexOptions, report, &data->source );
                if ( ASSETSTORE_STATUS_OK == report.status )
                    data->source.Complete( report.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, report.status, report.message );
                break;
            }
            case CPrimaryAssetStoreOperationData::OPERATION_VERIFY:
            {
                CAssetVerificationReport report;
                const TAssetStoreStatus status = store->VerifyLocation( data->locationId, report );
                if ( ASSETSTORE_STATUS_OK == status )
                    data->source.Complete( report.Clone(), true );
                else
                    CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, status, store->GetLastErrorMessage() );
                break;
            }
        }
    }
    catch ( ... )
    {
        CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, ASSETSTORE_STATUS_FAILED, "an exception ended the operation" );
    }
    store->OperationEnded();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStoreOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        CPrimaryAssetStoreOperationData* data = const_cast< CPrimaryAssetStoreOperationData* >( CORE::CICloneable::CloneableAs< CPrimaryAssetStoreOperationData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->store )
        {
            CPrimaryAssetStore* store = data->store;
            data->store = GUCEF_NULL;
            data->source.Cancel();
            store->OperationEnded();
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
