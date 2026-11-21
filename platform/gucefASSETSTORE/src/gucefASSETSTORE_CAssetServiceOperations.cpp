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

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

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

const CORE::CString CAssetServiceOperations::GetServiceInfo = "Info/GetServiceInfo";
const CORE::CString CAssetServiceOperations::GetCatalogueChanges = "Info/GetCatalogueChanges";
const CORE::CString CAssetServiceOperations::Search = "Catalogue/Search";
const CORE::CString CAssetServiceOperations::Suggest = "Catalogue/Suggest";
const CORE::CString CAssetServiceOperations::GetVocabulary = "Catalogue/GetVocabulary";
const CORE::CString CAssetServiceOperations::GetPack = "Catalogue/GetPack";
const CORE::CString CAssetServiceOperations::GetAssets = "Catalogue/GetAssets";
const CORE::CString CAssetServiceOperations::GetAsset = "Catalogue/GetAsset";
const CORE::CString CAssetServiceOperations::GetRelations = "Catalogue/GetRelations";
const CORE::CString CAssetServiceOperations::GetIngestHistory = "Catalogue/GetIngestHistory";
const CORE::CString CAssetServiceOperations::GetSavedSearches = "Catalogue/GetSavedSearches";
const CORE::CString CAssetServiceOperations::SaveSearch = "Catalogue/SaveSearch";
const CORE::CString CAssetServiceOperations::DeleteSavedSearch = "Catalogue/DeleteSavedSearch";
const CORE::CString CAssetServiceOperations::RunSavedSearch = "Catalogue/RunSavedSearch";
const CORE::CString CAssetServiceOperations::GetAssetFileSet = "Catalogue/GetAssetFileSet";
const CORE::CString CAssetServiceOperations::GetLicence = "Catalogue/GetLicence";
const CORE::CString CAssetServiceOperations::GetPackIndex = "Catalogue/GetPackIndex";
const CORE::CString CAssetServiceOperations::GetResourceGroups = "Catalogue/GetResourceGroups";
const CORE::CString CAssetServiceOperations::GetResources = "Catalogue/GetResources";
const CORE::CString CAssetServiceOperations::UpdatePackMetadata = "Curation/UpdatePackMetadata";
const CORE::CString CAssetServiceOperations::SetClassification = "Curation/SetClassification";
const CORE::CString CAssetServiceOperations::SetCategoryMapping = "Curation/SetCategoryMapping";
const CORE::CString CAssetServiceOperations::RemoveCategoryMapping = "Curation/RemoveCategoryMapping";
const CORE::CString CAssetServiceOperations::SetSynonym = "Curation/SetSynonym";
const CORE::CString CAssetServiceOperations::SetCategoryRule = "Curation/SetCategoryRule";
const CORE::CString CAssetServiceOperations::RemoveCategoryRule = "Curation/RemoveCategoryRule";
const CORE::CString CAssetServiceOperations::CountRuleMatches = "Curation/CountRuleMatches";
const CORE::CString CAssetServiceOperations::Reclassify = "Curation/Reclassify";
const CORE::CString CAssetServiceOperations::RemovePack = "Curation/RemovePack";
const CORE::CString CAssetServiceOperations::GetThumbnails = "Thumbnails/Get";
const CORE::CString CAssetServiceOperations::PutThumbnail = "Thumbnails/Put";
const CORE::CString CAssetServiceOperations::GetMissingThumbnails = "Thumbnails/GetMissing";
const CORE::CString CAssetServiceOperations::StatFiles = "Files/Stat";
const CORE::CString CAssetServiceOperations::ListDirectory = "Files/List";
const CORE::CString CAssetServiceOperations::ReadFileBlock = "Files/Read";
const CORE::CString CAssetServiceOperations::BeginUpload = "Files/BeginUpload";
const CORE::CString CAssetServiceOperations::BeginUploadFiles = "Files/BeginUploadFiles";
const CORE::CString CAssetServiceOperations::WriteUploadBlock = "Files/WriteUpload";
const CORE::CString CAssetServiceOperations::QueryUpload = "Files/QueryUpload";
const CORE::CString CAssetServiceOperations::CommitUpload = "Files/CommitUpload";
const CORE::CString CAssetServiceOperations::AbortUpload = "Files/AbortUpload";
const CORE::CString CAssetServiceOperations::GetLocations = "Locations/List";
const CORE::CString CAssetServiceOperations::RefreshLocationStates = "Locations/Refresh";
const CORE::CString CAssetServiceOperations::RelocateLocation = "Locations/Relocate";
const CORE::CString CAssetServiceOperations::VerifyLocation = "Locations/Verify";
const CORE::CString CAssetServiceOperations::Ingest = "Ingest/Ingest";
const CORE::CString CAssetServiceOperations::ResumeIngest = "Ingest/Resume";
const CORE::CString CAssetServiceOperations::GetPendingIngests = "Ingest/ListPending";
const CORE::CString CAssetServiceOperations::ListImportRoots = "Ingest/ListImportRoots";
const CORE::CString CAssetServiceOperations::SetSourcePolicy = "Ingest/SetSourcePolicy";
const CORE::CString CAssetServiceOperations::ListJobs = "Jobs/List";
const CORE::CString CAssetServiceOperations::GetJob = "Jobs/Get";
const CORE::CString CAssetServiceOperations::ControlJob = "Jobs/Control";
const CORE::CString CAssetServiceOperations::GetJobResult = "Jobs/GetResult";
const CORE::CString CAssetServiceOperations::ListStores = "Stores/List";
const CORE::CString CAssetServiceOperations::QueryLibrary = "Stores/QueryLibrary";
const CORE::CString CAssetServiceOperations::GetStoreItem = "Stores/GetItem";
const CORE::CString CAssetServiceOperations::EnqueueDownload = "Stores/EnqueueDownload";
const CORE::CString CAssetServiceOperations::BeginSignIn = "SignIn/Begin";
const CORE::CString CAssetServiceOperations::AnswerPrompt = "SignIn/Answer";
const CORE::CString CAssetServiceOperations::ImportWebSession = "SignIn/ImportSession";
const CORE::CString CAssetServiceOperations::CancelSignIn = "SignIn/Cancel";
const CORE::CString CAssetServiceOperations::SignOut = "SignIn/SignOut";
const CORE::CString CAssetServiceOperations::GetSignInPrompts = "SignIn/ListPrompts";
const CORE::CString CAssetServiceOperations::RequestConversion = "Conversion/Request";
const CORE::CString CAssetServiceOperations::RequestExport = "Conversion/Export";
const CORE::CString CAssetServiceOperations::RunPipelineQuery = "Pipeline/Query";
const CORE::CString CAssetServiceOperations::ApplyPipelineChange = "Pipeline/ApplyChange";
const CORE::CString CAssetServiceOperations::StartPipelineJob = "Pipeline/StartJob";
const CORE::CString CAssetServiceOperations::RenderSheet = "Pipeline/RenderSheet";
const CORE::CString CAssetServiceOperations::RequestLocalCopy = "LocalCopy/Request";
const CORE::CString CAssetServiceOperations::SetOfflinePin = "LocalCopy/SetOfflinePin";
const CORE::CString CAssetServiceOperations::UnlockKeyring = "Admin/UnlockKeyring";
const CORE::CString CAssetServiceOperations::LockKeyring = "Admin/LockKeyring";
const CORE::CString CAssetServiceOperations::GetAuditRecords = "Admin/GetAuditRecords";
const CORE::CString CAssetServiceOperations::ReindexFromStorage = "Admin/ReindexFromStorage";
const CORE::CString CAssetServiceOperations::GetResourceStatus = "Admin/GetResourceStatus";
const CORE::CString CAssetServiceOperations::GetSyncMarks = "Sync/GetMarks";
const CORE::CString CAssetServiceOperations::SetSyncMarks = "Sync/SetMarks";
const CORE::CString CAssetServiceOperations::GetSyncProfile = "Sync/GetProfile";
const CORE::CString CAssetServiceOperations::SetSyncProfile = "Sync/SetProfile";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CAssetServiceOperations::IsMutation( const CORE::CString& operation )
{GUCEF_TRACE;

    return operation == SaveSearch ||
           operation == DeleteSavedSearch ||
           operation == UpdatePackMetadata ||
           operation == SetClassification ||
           operation == SetCategoryMapping ||
           operation == RemoveCategoryMapping ||
           operation == SetSynonym ||
           operation == SetCategoryRule ||
           operation == RemoveCategoryRule ||
           operation == Reclassify ||
           operation == RemovePack ||
           operation == PutThumbnail ||
           operation == BeginUpload ||
           operation == BeginUploadFiles ||
           operation == WriteUploadBlock ||
           operation == CommitUpload ||
           operation == AbortUpload ||
           operation == RelocateLocation ||
           operation == VerifyLocation ||
           operation == Ingest ||
           operation == ResumeIngest ||
           operation == SetSourcePolicy ||
           operation == ControlJob ||
           operation == EnqueueDownload ||
           operation == BeginSignIn ||
           operation == AnswerPrompt ||
           operation == ImportWebSession ||
           operation == CancelSignIn ||
           operation == SignOut ||
           operation == RequestConversion ||
           operation == RequestExport ||
           operation == ApplyPipelineChange ||
           operation == StartPipelineJob ||
           operation == RequestLocalCopy ||
           operation == SetOfflinePin ||
           operation == UnlockKeyring ||
           operation == LockKeyring ||
           operation == ReindexFromStorage ||
           operation == SetSyncMarks ||
           operation == SetSyncProfile;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceOperations::IsKnown( const CORE::CString& operation )
{GUCEF_TRACE;

    return operation == GetServiceInfo ||
           operation == GetCatalogueChanges ||
           operation == Search ||
           operation == Suggest ||
           operation == GetVocabulary ||
           operation == GetPack ||
           operation == GetAssets ||
           operation == GetAsset ||
           operation == GetRelations ||
           operation == GetIngestHistory ||
           operation == GetSavedSearches ||
           operation == SaveSearch ||
           operation == DeleteSavedSearch ||
           operation == RunSavedSearch ||
           operation == GetAssetFileSet ||
           operation == GetLicence ||
           operation == GetPackIndex ||
           operation == GetResourceGroups ||
           operation == GetResources ||
           operation == UpdatePackMetadata ||
           operation == SetClassification ||
           operation == SetCategoryMapping ||
           operation == RemoveCategoryMapping ||
           operation == SetSynonym ||
           operation == SetCategoryRule ||
           operation == RemoveCategoryRule ||
           operation == CountRuleMatches ||
           operation == Reclassify ||
           operation == RemovePack ||
           operation == GetThumbnails ||
           operation == PutThumbnail ||
           operation == GetMissingThumbnails ||
           operation == StatFiles ||
           operation == ListDirectory ||
           operation == ReadFileBlock ||
           operation == BeginUpload ||
           operation == BeginUploadFiles ||
           operation == WriteUploadBlock ||
           operation == QueryUpload ||
           operation == CommitUpload ||
           operation == AbortUpload ||
           operation == GetLocations ||
           operation == RefreshLocationStates ||
           operation == RelocateLocation ||
           operation == VerifyLocation ||
           operation == Ingest ||
           operation == ResumeIngest ||
           operation == GetPendingIngests ||
           operation == ListImportRoots ||
           operation == SetSourcePolicy ||
           operation == ListJobs ||
           operation == GetJob ||
           operation == ControlJob ||
           operation == GetJobResult ||
           operation == ListStores ||
           operation == QueryLibrary ||
           operation == GetStoreItem ||
           operation == EnqueueDownload ||
           operation == BeginSignIn ||
           operation == AnswerPrompt ||
           operation == ImportWebSession ||
           operation == CancelSignIn ||
           operation == SignOut ||
           operation == GetSignInPrompts ||
           operation == RequestConversion ||
           operation == RequestExport ||
           operation == RunPipelineQuery ||
           operation == ApplyPipelineChange ||
           operation == StartPipelineJob ||
           operation == RenderSheet ||
           operation == RequestLocalCopy ||
           operation == SetOfflinePin ||
           operation == UnlockKeyring ||
           operation == LockKeyring ||
           operation == GetAuditRecords ||
           operation == ReindexFromStorage ||
           operation == GetResourceStatus ||
           operation == GetSyncMarks ||
           operation == SetSyncMarks ||
           operation == GetSyncProfile ||
           operation == SetSyncProfile;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
