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

#include <string.h>

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#include "gucefASSETSTORE_CAssetStoreRemoteCodec.h"

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

CORE::CIDataNodeSerializable*
CAssetStoreRemoteCodec::NewResult( const CORE::CString& operation )
{GUCEF_TRACE;

    typedef CAssetServiceOperations Op;

    // the records of the operations, as CIAssetStoreService::Dispatch() makes them
    if ( Op::GetServiceInfo == operation )
        return GUCEF_NEW CAssetServiceInfo();
    if ( Op::GetCatalogueChanges == operation )
        return GUCEF_NEW CAssetCatalogueChangePage();
    if ( Op::Search == operation || Op::RunSavedSearch == operation )
        return GUCEF_NEW CAssetSearchResultPage();
    if ( Op::Suggest == operation || Op::GetPendingIngests == operation || Op::ListImportRoots == operation )
        return GUCEF_NEW CAssetServiceStringList();
    if ( Op::GetVocabulary == operation )
        return GUCEF_NEW CAssetVocabularyInfo();
    if ( Op::GetPack == operation )
        return GUCEF_NEW CAssetPackRecord();
    if ( Op::GetAssets == operation )
        return GUCEF_NEW CAssetRecordPage();
    if ( Op::GetAsset == operation )
        return GUCEF_NEW CAssetRecord();
    if ( Op::GetRelations == operation )
        return GUCEF_NEW CAssetRelationPage();
    if ( Op::GetIngestHistory == operation )
        return GUCEF_NEW CAssetIngestHistoryPage();
    if ( Op::GetSavedSearches == operation )
        return GUCEF_NEW CAssetSavedSearchPage();
    if ( Op::GetAssetFileSet == operation )
        return GUCEF_NEW CAssetFileSet();
    if ( Op::GetLicence == operation )
        return GUCEF_NEW CAssetLicence();
    if ( Op::GetPackIndex == operation )
        return GUCEF_NEW CAssetPackIndexInfo();
    if ( Op::GetResourceGroups == operation )
        return GUCEF_NEW CAssetPackGroupPage();
    if ( Op::GetResources == operation )
        return GUCEF_NEW CAssetPackResourcePage();
    if ( Op::CountRuleMatches == operation || Op::GetJobResult == operation || Op::RunPipelineQuery == operation ||
         Op::ApplyPipelineChange == operation || Op::RenderSheet == operation || Op::GetAuditRecords == operation ||
         Op::GetResourceStatus == operation )
        return GUCEF_NEW CAssetServiceDocument();
    if ( Op::GetThumbnails == operation || Op::GetMissingThumbnails == operation )
        return GUCEF_NEW CAssetThumbnailPage();
    if ( Op::StatFiles == operation || Op::ListDirectory == operation || Op::QueryUpload == operation )
        return GUCEF_NEW CAssetFileStatPage();
    if ( Op::ReadFileBlock == operation )
        return GUCEF_NEW CAssetFileBlock();
    if ( Op::GetLocations == operation || Op::RefreshLocationStates == operation )
        return GUCEF_NEW CAssetLocationPage();
    if ( Op::VerifyLocation == operation )
        return GUCEF_NEW CAssetVerificationReport();
    if ( Op::Ingest == operation || Op::ResumeIngest == operation )
        return GUCEF_NEW CAssetIngestResult();
    if ( Op::ListJobs == operation )
        return GUCEF_NEW CAssetJobPage();
    if ( Op::GetJob == operation )
        return GUCEF_NEW CAssetJobEntry();
    if ( Op::ListStores == operation )
        return GUCEF_NEW CAssetStoreInfoPage();
    if ( Op::QueryLibrary == operation )
        return GUCEF_NEW CAssetStoreLibraryPage();
    if ( Op::GetStoreItem == operation )
        return GUCEF_NEW CAssetStoreItem();
    if ( Op::GetSignInPrompts == operation )
        return GUCEF_NEW CAssetSignInPromptPage();
    if ( Op::GetSyncMarks == operation )
        return GUCEF_NEW CAssetSyncMarkPage();
    if ( Op::GetSyncProfile == operation )
        return GUCEF_NEW CAssetLibrarySyncProfile();
    if ( Op::IsKnown( operation ) || Op::BeginUploadFiles == operation )
        return GUCEF_NEW CAssetServiceResult();      // every mutation that answers an outcome
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataNodeSerializable*
CAssetStoreRemoteCodec::AsRecord( const CORE::CString& operation, const CORE::CICloneable* result )
{GUCEF_TRACE;

    // the result of CIAssetStoreService::Dispatch() is the record type of the operation: checked by its type name, no RTTI
    if ( GUCEF_NULL == result )
        return GUCEF_NULL;
    CORE::CIDataNodeSerializable* probe = NewResult( operation );
    const bool same = GUCEF_NULL != probe && 0 == strcmp( probe->GetCloneableTypeName(), result->GetCloneableTypeName() );
    GUCEF_DELETE probe;
    return same ? static_cast< const CORE::CIDataNodeSerializable* >( result ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreRemoteCodec::FillAnswer( COMCORE::CRpcMessage& answer, TAssetStoreStatus status, const CORE::CString& message,
                                    const CORE::CIDataNodeSerializable* record, UInt64 eventSeq, bool replayed )
{GUCEF_TRACE;

    answer.SetString( CAssetStoreRemoteContracts::AnswerStatusField, AssetStoreStatusToString( status ) );
    answer.SetString( CAssetStoreRemoteContracts::AnswerMessageField, message );
    if ( GUCEF_NULL != record )
    {
        CORE::CDataNode node( "result", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNodeSerializableSettings settings;
        if ( record->Serialize( node, settings ) )
            answer.SetDataNode( CAssetStoreRemoteContracts::AnswerResultField, node );
    }
    answer.SetUInt64( CAssetStoreRemoteContracts::AnswerEventSeqField, eventSeq );
    answer.SetBool( CAssetStoreRemoteContracts::AnswerReplayedField, replayed );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreRemoteCodec::ReadAnswer( const CORE::CString& operation, const COMCORE::CRpcMessage& answer,
                                    CORE::CIDataNodeSerializable*& result, CORE::CString& message )
{GUCEF_TRACE;

    result = NewResult( operation );
    message = answer.GetString( CAssetStoreRemoteContracts::AnswerMessageField );
    const CORE::CString statusName = answer.GetString( CAssetStoreRemoteContracts::AnswerStatusField );
    const TAssetStoreStatus status = statusName.IsNULLOrEmpty() ? ASSETSTORE_STATUS_FAILED : AssetStoreStatusFromString( statusName );
    bool filled = false;
    if ( GUCEF_NULL != result && answer.Has( CAssetStoreRemoteContracts::AnswerResultField ) )
    {
        CORE::CDataNode node;
        CORE::CDataNodeSerializableSettings settings;
        filled = answer.GetDataNode( CAssetStoreRemoteContracts::AnswerResultField, node ) && result->Deserialize( node, settings );
    }
    if ( !filled && ASSETSTORE_STATUS_OK != status )
        SetFailure( result, status, message );
    // the server's replay of a command answers the stored outcome: it says so in the record as the local service does
    if ( answer.GetBool( CAssetStoreRemoteContracts::AnswerReplayedField ) )
    {
        CAssetServiceResult* outcome = CORE::CICloneable::CloneableAs< CAssetServiceResult >( result );
        if ( GUCEF_NULL != outcome )
            outcome->replayed = true;
    }
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreRemoteCodec::StatusOfRpcFailure( const COMCORE::CRpcStatus& status )
{GUCEF_TRACE;

    switch ( status.code )
    {
        case COMCORE::RPC_STATUS_OK:                    return ASSETSTORE_STATUS_OK;
        case COMCORE::RPC_STATUS_CANCELLED:             return ASSETSTORE_STATUS_CANCELLED;
        case COMCORE::RPC_STATUS_INVALID_ARGUMENT:      return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        case COMCORE::RPC_STATUS_OUT_OF_RANGE:          return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        case COMCORE::RPC_STATUS_DEADLINE_EXCEEDED:     return ASSETSTORE_STATUS_TIMED_OUT;
        case COMCORE::RPC_STATUS_NOT_FOUND:             return ASSETSTORE_STATUS_NOT_FOUND;
        case COMCORE::RPC_STATUS_ALREADY_EXISTS:        return ASSETSTORE_STATUS_ALREADY_EXISTS;
        case COMCORE::RPC_STATUS_PERMISSION_DENIED:     return ASSETSTORE_STATUS_REFUSED;
        case COMCORE::RPC_STATUS_UNAUTHENTICATED:       return ASSETSTORE_STATUS_REFUSED;
        case COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED:    return ASSETSTORE_STATUS_RATE_LIMITED;
        case COMCORE::RPC_STATUS_UNIMPLEMENTED:         return ASSETSTORE_STATUS_UNSUPPORTED;
        case COMCORE::RPC_STATUS_UNAVAILABLE:           return ASSETSTORE_STATUS_UNAVAILABLE;
        case COMCORE::RPC_STATUS_DATA_LOSS:             return ASSETSTORE_STATUS_IO_ERROR;
        case COMCORE::RPC_STATUS_FAILED_PRECONDITION:
        case COMCORE::RPC_STATUS_ABORTED:
        case COMCORE::RPC_STATUS_INTERNAL:
        case COMCORE::RPC_STATUS_UNKNOWN:
        default:                                        return ASSETSTORE_STATUS_NETWORK_ERROR;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreRemoteCodec::SetFailure( CORE::CICloneable* result, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    if ( GUCEF_NULL == result )
        return false;
    CAssetServiceResult* outcome = CORE::CICloneable::CloneableAs< CAssetServiceResult >( result );
    if ( GUCEF_NULL != outcome )
    {
        outcome->status = status;
        outcome->message = message;
        return true;
    }
    CAssetServiceDocument* document = CORE::CICloneable::CloneableAs< CAssetServiceDocument >( result );
    if ( GUCEF_NULL != document )
    {
        document->status = status;
        document->message = message;
        return true;
    }
    CAssetIngestResult* ingest = CORE::CICloneable::CloneableAs< CAssetIngestResult >( result );
    if ( GUCEF_NULL != ingest )
    {
        ingest->status = status;
        ingest->message = message;
        return true;
    }
    CAssetFileSet* fileSet = CORE::CICloneable::CloneableAs< CAssetFileSet >( result );
    if ( GUCEF_NULL != fileSet )
    {
        fileSet->mainStatus = status;
        fileSet->mainMessage = message;
        return true;
    }
    CAssetVerificationReport* report = CORE::CICloneable::CloneableAs< CAssetVerificationReport >( result );
    if ( GUCEF_NULL != report )
    {
        report->status = status;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
