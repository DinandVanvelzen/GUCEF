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

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

const CORE::CEvent CIAssetStoreService::CatalogueChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::CatalogueChangedEvent";
const CORE::CEvent CIAssetStoreService::JobChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::JobChangedEvent";
const CORE::CEvent CIAssetStoreService::JobRemovedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::JobRemovedEvent";
const CORE::CEvent CIAssetStoreService::LocationStateChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::LocationStateChangedEvent";
const CORE::CEvent CIAssetStoreService::StoreStateChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::StoreStateChangedEvent";
const CORE::CEvent CIAssetStoreService::StoresChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::StoresChangedEvent";
const CORE::CEvent CIAssetStoreService::SignInPromptEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::SignInPromptEvent";
const CORE::CEvent CIAssetStoreService::SignInPromptClosedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::SignInPromptClosedEvent";
const CORE::CEvent CIAssetStoreService::ThumbnailUpdatedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::ThumbnailUpdatedEvent";
const CORE::CEvent CIAssetStoreService::ListingImageReadyEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::ListingImageReadyEvent";
const CORE::CEvent CIAssetStoreService::NoticeEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::NoticeEvent";
const CORE::CEvent CIAssetStoreService::ConnectionStateChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreService::ConnectionStateChangedEvent";

/*-------------------------------------------------------------------------*/

void
CIAssetStoreService::RegisterEvents( void )
{GUCEF_TRACE;

    CatalogueChangedEvent.Initialize();
    JobChangedEvent.Initialize();
    JobRemovedEvent.Initialize();
    LocationStateChangedEvent.Initialize();
    StoreStateChangedEvent.Initialize();
    StoresChangedEvent.Initialize();
    SignInPromptEvent.Initialize();
    SignInPromptClosedEvent.Initialize();
    ThumbnailUpdatedEvent.Initialize();
    ListingImageReadyEvent.Initialize();
    NoticeEvent.Initialize();
    ConnectionStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIAssetStoreService::CIAssetStoreService( CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIAssetStoreService::~CIAssetStoreService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIAssetStoreService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CIAssetStoreService";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CIAssetStoreService::ClampPageEntries( UInt32 maxEntries )
{GUCEF_TRACE;

    if ( 0 == maxEntries )
        return DefaultPageEntries;
    return maxEntries > MaxPageEntries ? MaxPageEntries : maxEntries;
}

/*-------------------------------------------------------------------------*/

UInt32
CIAssetStoreService::ParsePageToken( const CORE::CString& pageToken )
{GUCEF_TRACE;

    return pageToken.IsNULLOrEmpty() ? 0 : CORE::StringToUInt32( pageToken, 0 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreService::MakePageToken( UInt32 offset )
{GUCEF_TRACE;

    return CORE::UInt32ToString( offset );
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreService::IsOutcomeStatus( const CORE::CString& operation, TAssetStoreStatus status )
{GUCEF_TRACE;

    if ( ASSETSTORE_STATUS_OK == status )
        return true;
    // a duplicate is an outcome of an ingest, not a failure: the result tells
    return ASSETSTORE_STATUS_DUPLICATE == status && ( CAssetServiceOperations::Ingest == operation || CAssetServiceOperations::ResumeIngest == operation );
}

/*-------------------------------------------------------------------------*/

void
CIAssetStoreService::CompleteCall( CORE::CFutureResultSource& source, const CAssetServiceCall& call, TAssetStoreStatus status,
                                   CORE::CICloneable* result, const CORE::CString& message )
{GUCEF_TRACE;

    if ( IsOutcomeStatus( call.operation, status ) && GUCEF_NULL != result )
    {
        source.Complete( result, true );
        return;
    }
    GUCEF_DELETE result;
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

CORE::CFutureResult
CIAssetStoreService::MakeEndedFuture( TAssetStoreStatus status, CORE::CICloneable* result, const CORE::CString& message )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    CAssetServiceCall call;
    CompleteCall( source, call, status, result, message );
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CIAssetStoreService::GetStatusOfFuture( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    return CIAssetStoreProvider::GetStatusOfFuture( future );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreService::GetMessageOfFuture( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !future.IsReady() )
        return CORE::CString::Empty;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() || CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() )
        return CORE::CString::Empty;
    const CORE::CString info = task->GetTaskStatusExtraInfo();
    const Int32 colon = info.HasChar( ':', true );
    return colon < 0 ? CORE::CString::Empty : info.CutChars( (UInt32) colon + 1, true ).Trim( true );
}

/*-------------------------------------------------------------------------*/

namespace {

/** The message a result object carries, for the failure of a call */
CORE::CString
MessageOf( const CORE::CICloneable* result )
{
    const CAssetServiceResult* outcome = CORE::CICloneable::CloneableAs< CAssetServiceResult >( result );
    if ( GUCEF_NULL != outcome )
        return outcome->message;
    const CAssetServiceDocument* document = CORE::CICloneable::CloneableAs< CAssetServiceDocument >( result );
    if ( GUCEF_NULL != document )
        return document->message;
    const CAssetIngestResult* ingest = CORE::CICloneable::CloneableAs< CAssetIngestResult >( result );
    if ( GUCEF_NULL != ingest )
        return ingest->message;
    const CAssetFileSet* fileSet = CORE::CICloneable::CloneableAs< CAssetFileSet >( result );
    if ( GUCEF_NULL != fileSet )
        return fileSet->mainMessage;
    return CORE::CString::Empty;
}

}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetServiceInfoAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetServiceInfo );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetCatalogueChangesAsync( UInt64 sinceRevision, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetCatalogueChanges );
    call.SetUInt64( "sinceRevision", sinceRevision );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SearchAsync( const CAssetSearchQuery& query )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Search );
    call.SetRecord( "query", query );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SuggestAsync( const CORE::CString& prefix, UInt32 maxCount )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Suggest );
    call.SetText( "prefix", prefix );
    call.SetUInt32( "maxCount", maxCount );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetVocabularyAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetVocabulary );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetPackAsync( const CORE::CString& packId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetPack );
    call.SetText( "packId", packId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetAssetsAsync( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAssets );
    call.SetText( "packId", packId );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetAssetAsync( const CORE::CString& assetId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAsset );
    call.SetText( "assetId", assetId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetRelationsAsync( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetRelations );
    call.SetText( "assetId", assetId );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetIngestHistoryAsync( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetIngestHistory );
    call.SetText( "packId", packId );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetSavedSearchesAsync( const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetSavedSearches );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SaveSearchAsync( const CORE::CString& commandId, const CAssetSavedSearch& search )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SaveSearch, commandId );
    call.SetRecord( "search", search );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::DeleteSavedSearchAsync( const CORE::CString& commandId, const CORE::CString& searchId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::DeleteSavedSearch, commandId );
    call.SetText( "searchId", searchId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RunSavedSearchAsync( const CORE::CString& searchId, UInt32 pageIndex )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RunSavedSearch );
    call.SetText( "searchId", searchId );
    call.SetUInt32( "pageIndex", pageIndex );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetAssetFileSetAsync( const CORE::CString& assetId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAssetFileSet );
    call.SetText( "assetId", assetId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetLicenceAsync( const CORE::CString& licenceOrPackId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetLicence );
    call.SetText( "licenceOrPackId", licenceOrPackId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::UpdatePackMetadataAsync( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::UpdatePackMetadata, commandId );
    call.SetRecord( "pack", pack );
    call.SetUInt64( "expectedRevision", expectedRevision );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SetClassificationAsync( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetClassification, commandId );
    call.SetText( "objectId", objectId );
    call.SetRecord( "classification", classification );
    call.SetText( "source", source );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SetCategoryMappingAsync( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetCategoryMapping, commandId );
    call.SetText( "sourceId", sourceId );
    call.SetText( "storeCategory", storeCategory );
    call.SetText( "categoryId", categoryId );
    call.SetText( "theme", theme );
    call.SetText( "kind", kind );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RemoveCategoryMappingAsync( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RemoveCategoryMapping, commandId );
    call.SetText( "sourceId", sourceId );
    call.SetText( "storeCategory", storeCategory );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SetSynonymAsync( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetSynonym, commandId );
    call.SetText( "word", word );
    call.SetText( "replacement", replacement );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SetCategoryRuleAsync( const CORE::CString& commandId, const CAssetCategoryRule& rule )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetCategoryRule, commandId );
    call.SetRecord( "rule", rule );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RemoveCategoryRuleAsync( const CORE::CString& commandId, const CORE::CString& ruleId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RemoveCategoryRule, commandId );
    call.SetText( "ruleId", ruleId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::CountRuleMatchesAsync( const CAssetCategoryRule& rule )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::CountRuleMatches );
    call.SetRecord( "rule", rule );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ReclassifyAsync( const CORE::CString& commandId, const CORE::CString::StringVector& packIds )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Reclassify, commandId );
    call.SetTexts( "packIds", packIds );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RemovePackAsync( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RemovePack, commandId );
    call.SetText( "packId", packId );
    call.SetBool( "deleteFiles", deleteFiles );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetThumbnailsAsync( const CORE::CString::StringVector& assetIds, UInt32 size )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetThumbnails );
    call.SetTexts( "assetIds", assetIds );
    call.SetUInt32( "size", size );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::PutThumbnailAsync( const CORE::CString& commandId, const CAssetThumbnail& thumbnail )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::PutThumbnail, commandId );
    call.SetRecord( "thumbnail", thumbnail );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetMissingThumbnailsAsync( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetMissingThumbnails );
    call.SetText( "rendererId", rendererId );
    call.SetText( "rendererVersion", rendererVersion );
    call.SetUInt32( "size", size );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::StatFilesAsync( const CORE::CString::StringVector& vfsPaths )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::StatFiles );
    call.SetTexts( "vfsPaths", vfsPaths );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ListDirectoryAsync( const CORE::CString& vfsPath, const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListDirectory );
    call.SetText( "vfsPath", vfsPath );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ReadFileBlockAsync( const CORE::CString& vfsPath, UInt64 offset, UInt32 length )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ReadFileBlock );
    call.SetText( "vfsPath", vfsPath );
    call.SetUInt64( "offset", offset );
    call.SetUInt32( "length", length );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::BeginUploadAsync( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::BeginUpload, commandId );
    call.SetText( "name", name );
    call.SetUInt64( "totalBytes", totalBytes );
    call.SetUInt32( "fileCount", fileCount );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::QueryUploadAsync( const CORE::CString& uploadId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::QueryUpload );
    call.SetText( "uploadId", uploadId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::CommitUploadAsync( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::CommitUpload, commandId );
    call.SetText( "uploadId", uploadId );
    call.SetRecord( "ingest", ingest );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::AbortUploadAsync( const CORE::CString& commandId, const CORE::CString& uploadId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::AbortUpload, commandId );
    call.SetText( "uploadId", uploadId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetLocationsAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetLocations );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RefreshLocationStatesAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RefreshLocationStates );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RelocateLocationAsync( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RelocateLocation, commandId );
    call.SetText( "locationId", locationId );
    call.SetText( "newVfsRoot", newVfsRoot );
    call.SetText( "newFileSystemPath", newFileSystemPath );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::VerifyLocationAsync( const CORE::CString& commandId, const CORE::CString& locationId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::VerifyLocation, commandId );
    call.SetText( "locationId", locationId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::IngestAsync( const CORE::CString& commandId, const CAssetIngestRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Ingest, commandId );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ResumeIngestAsync( const CORE::CString& commandId, const CORE::CString& ingestId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ResumeIngest, commandId );
    call.SetText( "ingestId", ingestId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetPendingIngestsAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetPendingIngests );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ListImportRootsAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListImportRoots );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SetSourcePolicyAsync( const CORE::CString& commandId, const CAssetSourcePolicy& policy )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetSourcePolicy, commandId );
    call.SetRecord( "policy", policy );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ListJobsAsync( const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListJobs );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetJobAsync( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetJob );
    call.SetText( "jobId", jobId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ControlJobAsync( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ControlJob, commandId );
    call.SetText( "jobId", jobId );
    call.SetText( "action", AssetJobActionToString( action ) );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetJobResultAsync( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetJobResult );
    call.SetText( "jobId", jobId );
    call.SetText( "section", section );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ListStoresAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListStores );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::QueryLibraryAsync( const CORE::CString& storeId, const CAssetStoreQuery& query )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::QueryLibrary );
    call.SetText( "storeId", storeId );
    call.SetRecord( "query", query );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetStoreItemAsync( const CORE::CString& storeId, const CORE::CString& itemId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetStoreItem );
    call.SetText( "storeId", storeId );
    call.SetText( "itemId", itemId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::EnqueueDownloadAsync( const CORE::CString& commandId, const CAssetDownloadRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::EnqueueDownload, commandId );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::BeginSignInAsync( const CORE::CString& commandId, const CORE::CString& storeId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::BeginSignIn, commandId );
    call.SetText( "storeId", storeId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::CancelSignInAsync( const CORE::CString& commandId, const CORE::CString& storeId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::CancelSignIn, commandId );
    call.SetText( "storeId", storeId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SignOutAsync( const CORE::CString& commandId, const CORE::CString& storeId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SignOut, commandId );
    call.SetText( "storeId", storeId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetSignInPromptsAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetSignInPrompts );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RequestConversionAsync( const CORE::CString& commandId, const CAssetConversionJobRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RequestConversion, commandId );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RequestExportAsync( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RequestExport, commandId );
    call.SetTexts( "assetIds", assetIds );
    call.SetText( "profile", profile );
    call.SetText( "format", format );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RunPipelineQueryAsync( const CAssetPipelineRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RunPipelineQuery );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ApplyPipelineChangeAsync( const CORE::CString& commandId, const CAssetPipelineRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ApplyPipelineChange, commandId );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::StartPipelineJobAsync( const CORE::CString& commandId, const CAssetPipelineRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::StartPipelineJob, commandId );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RenderSheetAsync( const CAssetPipelineRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RenderSheet );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::RequestLocalCopyAsync( const CORE::CString& commandId, const CAssetLocalCopyRequest& request )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RequestLocalCopy, commandId );
    call.SetRecord( "request", request );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::SetOfflinePinAsync( const CORE::CString& commandId, const CORE::CString& packId, bool pinned )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetOfflinePin, commandId );
    call.SetText( "packId", packId );
    call.SetBool( "pinned", pinned );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::LockKeyringAsync( const CORE::CString& commandId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::LockKeyring, commandId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetAuditRecordsAsync( const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAuditRecords );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::ReindexFromStorageAsync( const CORE::CString& commandId )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ReindexFromStorage, commandId );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreService::GetResourceStatusAsync(  )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetResourceStatus );
    return ExecuteAsync( call );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CIAssetStoreService::Dispatch( const CAssetServiceCall& call, CORE::CICloneable*& result, CORE::CString& message )
{GUCEF_TRACE;

    result = GUCEF_NULL;
    message.Clear();
    const CORE::CString& commandId = call.commandId;
    const CORE::CString& operation = call.operation;
    TAssetStoreStatus status = ASSETSTORE_STATUS_UNSUPPORTED;
    if ( CAssetServiceOperations::GetServiceInfo == operation )
    {
        CAssetServiceInfo* answer = GUCEF_NEW CAssetServiceInfo();
        status = GetServiceInfo( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetCatalogueChanges == operation )
    {
        const UInt64 sinceRevision = call.GetUInt64( "sinceRevision" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetCatalogueChangePage* answer = GUCEF_NEW CAssetCatalogueChangePage();
        status = GetCatalogueChanges( sinceRevision, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::Search == operation )
    {
        CAssetSearchQuery query;
        call.GetRecord( "query", query );
        CAssetSearchResultPage* answer = GUCEF_NEW CAssetSearchResultPage();
        status = Search( query, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::Suggest == operation )
    {
        const CORE::CString prefix = call.GetText( "prefix" );
        const UInt32 maxCount = call.GetUInt32( "maxCount" );
        CAssetServiceStringList* answer = GUCEF_NEW CAssetServiceStringList();
        status = Suggest( prefix, maxCount, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetVocabulary == operation )
    {
        CAssetVocabularyInfo* answer = GUCEF_NEW CAssetVocabularyInfo();
        status = GetVocabulary( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetPack == operation )
    {
        const CORE::CString packId = call.GetText( "packId" );
        CAssetPackRecord* answer = GUCEF_NEW CAssetPackRecord();
        status = GetPack( packId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetAssets == operation )
    {
        const CORE::CString packId = call.GetText( "packId" );
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetRecordPage* answer = GUCEF_NEW CAssetRecordPage();
        status = GetAssets( packId, pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetAsset == operation )
    {
        const CORE::CString assetId = call.GetText( "assetId" );
        CAssetRecord* answer = GUCEF_NEW CAssetRecord();
        status = GetAsset( assetId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetRelations == operation )
    {
        const CORE::CString assetId = call.GetText( "assetId" );
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetRelationPage* answer = GUCEF_NEW CAssetRelationPage();
        status = GetRelations( assetId, pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetIngestHistory == operation )
    {
        const CORE::CString packId = call.GetText( "packId" );
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetIngestHistoryPage* answer = GUCEF_NEW CAssetIngestHistoryPage();
        status = GetIngestHistory( packId, pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetSavedSearches == operation )
    {
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetSavedSearchPage* answer = GUCEF_NEW CAssetSavedSearchPage();
        status = GetSavedSearches( pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SaveSearch == operation )
    {
        CAssetSavedSearch search;
        call.GetRecord( "search", search );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SaveSearch( commandId, search, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::DeleteSavedSearch == operation )
    {
        const CORE::CString searchId = call.GetText( "searchId" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = DeleteSavedSearch( commandId, searchId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RunSavedSearch == operation )
    {
        const CORE::CString searchId = call.GetText( "searchId" );
        const UInt32 pageIndex = call.GetUInt32( "pageIndex" );
        CAssetSearchResultPage* answer = GUCEF_NEW CAssetSearchResultPage();
        status = RunSavedSearch( searchId, pageIndex, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetAssetFileSet == operation )
    {
        const CORE::CString assetId = call.GetText( "assetId" );
        CAssetFileSet* answer = GUCEF_NEW CAssetFileSet();
        status = GetAssetFileSet( assetId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetLicence == operation )
    {
        const CORE::CString licenceOrPackId = call.GetText( "licenceOrPackId" );
        CAssetLicence* answer = GUCEF_NEW CAssetLicence();
        status = GetLicence( licenceOrPackId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::UpdatePackMetadata == operation )
    {
        CAssetPackRecord pack;
        call.GetRecord( "pack", pack );
        const UInt64 expectedRevision = call.GetUInt64( "expectedRevision" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = UpdatePackMetadata( commandId, pack, expectedRevision, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SetClassification == operation )
    {
        const CORE::CString objectId = call.GetText( "objectId" );
        CAssetClassification classification;
        call.GetRecord( "classification", classification );
        const CORE::CString source = call.GetText( "source" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SetClassification( commandId, objectId, classification, source, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SetCategoryMapping == operation )
    {
        const CORE::CString sourceId = call.GetText( "sourceId" );
        const CORE::CString storeCategory = call.GetText( "storeCategory" );
        const CORE::CString categoryId = call.GetText( "categoryId" );
        const CORE::CString theme = call.GetText( "theme" );
        const CORE::CString kind = call.GetText( "kind" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SetCategoryMapping( commandId, sourceId, storeCategory, categoryId, theme, kind, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RemoveCategoryMapping == operation )
    {
        const CORE::CString sourceId = call.GetText( "sourceId" );
        const CORE::CString storeCategory = call.GetText( "storeCategory" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RemoveCategoryMapping( commandId, sourceId, storeCategory, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SetSynonym == operation )
    {
        const CORE::CString word = call.GetText( "word" );
        const CORE::CString replacement = call.GetText( "replacement" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SetSynonym( commandId, word, replacement, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SetCategoryRule == operation )
    {
        CAssetCategoryRule rule;
        call.GetRecord( "rule", rule );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SetCategoryRule( commandId, rule, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RemoveCategoryRule == operation )
    {
        const CORE::CString ruleId = call.GetText( "ruleId" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RemoveCategoryRule( commandId, ruleId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::CountRuleMatches == operation )
    {
        CAssetCategoryRule rule;
        call.GetRecord( "rule", rule );
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = CountRuleMatches( rule, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::Reclassify == operation )
    {
        const CORE::CString::StringVector packIds = call.GetTexts( "packIds" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = Reclassify( commandId, packIds, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RemovePack == operation )
    {
        const CORE::CString packId = call.GetText( "packId" );
        const bool deleteFiles = call.GetBool( "deleteFiles" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RemovePack( commandId, packId, deleteFiles, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetThumbnails == operation )
    {
        const CORE::CString::StringVector assetIds = call.GetTexts( "assetIds" );
        const UInt32 size = call.GetUInt32( "size" );
        CAssetThumbnailPage* answer = GUCEF_NEW CAssetThumbnailPage();
        status = GetThumbnails( assetIds, size, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::PutThumbnail == operation )
    {
        CAssetThumbnail thumbnail;
        call.GetRecord( "thumbnail", thumbnail );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = PutThumbnail( commandId, thumbnail, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetMissingThumbnails == operation )
    {
        const CORE::CString rendererId = call.GetText( "rendererId" );
        const CORE::CString rendererVersion = call.GetText( "rendererVersion" );
        const UInt32 size = call.GetUInt32( "size" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetThumbnailPage* answer = GUCEF_NEW CAssetThumbnailPage();
        status = GetMissingThumbnails( rendererId, rendererVersion, size, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::StatFiles == operation )
    {
        const CORE::CString::StringVector vfsPaths = call.GetTexts( "vfsPaths" );
        CAssetFileStatPage* answer = GUCEF_NEW CAssetFileStatPage();
        status = StatFiles( vfsPaths, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ListDirectory == operation )
    {
        const CORE::CString vfsPath = call.GetText( "vfsPath" );
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetFileStatPage* answer = GUCEF_NEW CAssetFileStatPage();
        status = ListDirectory( vfsPath, pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ReadFileBlock == operation )
    {
        const CORE::CString vfsPath = call.GetText( "vfsPath" );
        const UInt64 offset = call.GetUInt64( "offset" );
        const UInt32 length = call.GetUInt32( "length" );
        CAssetFileBlock* answer = GUCEF_NEW CAssetFileBlock();
        status = ReadFileBlock( vfsPath, offset, length, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::BeginUpload == operation )
    {
        const CORE::CString name = call.GetText( "name" );
        const UInt64 totalBytes = call.GetUInt64( "totalBytes" );
        const UInt32 fileCount = call.GetUInt32( "fileCount" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = BeginUpload( commandId, name, totalBytes, fileCount, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::QueryUpload == operation )
    {
        const CORE::CString uploadId = call.GetText( "uploadId" );
        CAssetFileStatPage* answer = GUCEF_NEW CAssetFileStatPage();
        status = QueryUpload( uploadId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::CommitUpload == operation )
    {
        const CORE::CString uploadId = call.GetText( "uploadId" );
        CAssetIngestRequest ingest;
        call.GetRecord( "ingest", ingest );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = CommitUpload( commandId, uploadId, ingest, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::AbortUpload == operation )
    {
        const CORE::CString uploadId = call.GetText( "uploadId" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = AbortUpload( commandId, uploadId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetLocations == operation )
    {
        CAssetLocationPage* answer = GUCEF_NEW CAssetLocationPage();
        status = GetLocations( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RefreshLocationStates == operation )
    {
        CAssetLocationPage* answer = GUCEF_NEW CAssetLocationPage();
        status = RefreshLocationStates( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RelocateLocation == operation )
    {
        const CORE::CString locationId = call.GetText( "locationId" );
        const CORE::CString newVfsRoot = call.GetText( "newVfsRoot" );
        const CORE::CString newFileSystemPath = call.GetText( "newFileSystemPath" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RelocateLocation( commandId, locationId, newVfsRoot, newFileSystemPath, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::VerifyLocation == operation )
    {
        const CORE::CString locationId = call.GetText( "locationId" );
        CAssetVerificationReport* answer = GUCEF_NEW CAssetVerificationReport();
        status = VerifyLocation( commandId, locationId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::Ingest == operation )
    {
        CAssetIngestRequest request;
        call.GetRecord( "request", request );
        CAssetIngestResult* answer = GUCEF_NEW CAssetIngestResult();
        status = Ingest( commandId, request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ResumeIngest == operation )
    {
        const CORE::CString ingestId = call.GetText( "ingestId" );
        CAssetIngestResult* answer = GUCEF_NEW CAssetIngestResult();
        status = ResumeIngest( commandId, ingestId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetPendingIngests == operation )
    {
        CAssetServiceStringList* answer = GUCEF_NEW CAssetServiceStringList();
        status = GetPendingIngests( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ListImportRoots == operation )
    {
        CAssetServiceStringList* answer = GUCEF_NEW CAssetServiceStringList();
        status = ListImportRoots( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SetSourcePolicy == operation )
    {
        CAssetSourcePolicy policy;
        call.GetRecord( "policy", policy );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SetSourcePolicy( commandId, policy, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ListJobs == operation )
    {
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetJobPage* answer = GUCEF_NEW CAssetJobPage();
        status = ListJobs( pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetJob == operation )
    {
        const CORE::CString jobId = call.GetText( "jobId" );
        CAssetJobEntry* answer = GUCEF_NEW CAssetJobEntry();
        status = GetJob( jobId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ControlJob == operation )
    {
        const CORE::CString jobId = call.GetText( "jobId" );
        TAssetJobAction action = ASSETJOB_ACTION_REMOVE;
        if ( !AssetJobActionFromString( call.GetText( "action" ), action ) )
        {
            message = "The action \"" + call.GetText( "action" ) + "\" is not known";
            return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        }
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = ControlJob( commandId, jobId, action, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetJobResult == operation )
    {
        const CORE::CString jobId = call.GetText( "jobId" );
        const CORE::CString section = call.GetText( "section" );
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = GetJobResult( jobId, section, pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ListStores == operation )
    {
        CAssetStoreInfoPage* answer = GUCEF_NEW CAssetStoreInfoPage();
        status = ListStores( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::QueryLibrary == operation )
    {
        const CORE::CString storeId = call.GetText( "storeId" );
        CAssetStoreQuery query;
        call.GetRecord( "query", query );
        CAssetStoreLibraryPage* answer = GUCEF_NEW CAssetStoreLibraryPage();
        status = QueryLibrary( storeId, query, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetStoreItem == operation )
    {
        const CORE::CString storeId = call.GetText( "storeId" );
        const CORE::CString itemId = call.GetText( "itemId" );
        CAssetStoreItem* answer = GUCEF_NEW CAssetStoreItem();
        status = GetStoreItem( storeId, itemId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::EnqueueDownload == operation )
    {
        CAssetDownloadRequest request;
        call.GetRecord( "request", request );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = EnqueueDownload( commandId, request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::BeginSignIn == operation )
    {
        const CORE::CString storeId = call.GetText( "storeId" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = BeginSignIn( commandId, storeId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::CancelSignIn == operation )
    {
        const CORE::CString storeId = call.GetText( "storeId" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = CancelSignIn( commandId, storeId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SignOut == operation )
    {
        const CORE::CString storeId = call.GetText( "storeId" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SignOut( commandId, storeId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetSignInPrompts == operation )
    {
        CAssetSignInPromptPage* answer = GUCEF_NEW CAssetSignInPromptPage();
        status = GetSignInPrompts( *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RequestConversion == operation )
    {
        CAssetConversionJobRequest request;
        call.GetRecord( "request", request );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RequestConversion( commandId, request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RequestExport == operation )
    {
        const CORE::CString::StringVector assetIds = call.GetTexts( "assetIds" );
        const CORE::CString profile = call.GetText( "profile" );
        const CORE::CString format = call.GetText( "format" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RequestExport( commandId, assetIds, profile, format, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RunPipelineQuery == operation )
    {
        CAssetPipelineRequest request;
        call.GetRecord( "request", request );
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = RunPipelineQuery( request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ApplyPipelineChange == operation )
    {
        CAssetPipelineRequest request;
        call.GetRecord( "request", request );
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = ApplyPipelineChange( commandId, request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::StartPipelineJob == operation )
    {
        CAssetPipelineRequest request;
        call.GetRecord( "request", request );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = StartPipelineJob( commandId, request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RenderSheet == operation )
    {
        CAssetPipelineRequest request;
        call.GetRecord( "request", request );
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = RenderSheet( request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::RequestLocalCopy == operation )
    {
        CAssetLocalCopyRequest request;
        call.GetRecord( "request", request );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = RequestLocalCopy( commandId, request, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::SetOfflinePin == operation )
    {
        const CORE::CString packId = call.GetText( "packId" );
        const bool pinned = call.GetBool( "pinned" );
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = SetOfflinePin( commandId, packId, pinned, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::LockKeyring == operation )
    {
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = LockKeyring( commandId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetAuditRecords == operation )
    {
        const CORE::CString pageToken = call.GetText( "pageToken" );
        const UInt32 maxEntries = call.GetUInt32( "maxEntries" );
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = GetAuditRecords( pageToken, maxEntries, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::ReindexFromStorage == operation )
    {
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        status = ReindexFromStorage( commandId, *answer );
        result = answer;
    }
    else
    if ( CAssetServiceOperations::GetResourceStatus == operation )
    {
        CAssetServiceDocument* answer = GUCEF_NEW CAssetServiceDocument();
        status = GetResourceStatus( *answer );
        result = answer;
    }
    else
    {
        message = "The operation \"" + operation + "\" is not known";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    if ( ASSETSTORE_STATUS_OK != status )
        message = MessageOf( result );
    return status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
