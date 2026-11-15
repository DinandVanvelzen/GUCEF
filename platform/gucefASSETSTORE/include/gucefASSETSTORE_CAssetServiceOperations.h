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

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The names of the operations of the asset store service (CIAssetStoreService), one per method, in the form
 *  "<group>/<method>" of the method groups of section 4.2 of the service design. They are the operation of a
 *  CAssetServiceCall (the asynchronous forms and a remote transport carry them), the names a recording service keeps
 *  and the method names of the remote contract.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetServiceOperations
{
    public:

    // Info
    static const CORE::CString GetServiceInfo;              /**< "Info/GetServiceInfo" */
    static const CORE::CString GetCatalogueChanges;         /**< "Info/GetCatalogueChanges" */

    // Catalogue
    static const CORE::CString Search;                      /**< "Catalogue/Search" */
    static const CORE::CString Suggest;                     /**< "Catalogue/Suggest" */
    static const CORE::CString GetVocabulary;               /**< "Catalogue/GetVocabulary" */
    static const CORE::CString GetPack;                     /**< "Catalogue/GetPack" */
    static const CORE::CString GetAssets;                   /**< "Catalogue/GetAssets" */
    static const CORE::CString GetAsset;                    /**< "Catalogue/GetAsset" */
    static const CORE::CString GetRelations;                /**< "Catalogue/GetRelations" */
    static const CORE::CString GetIngestHistory;            /**< "Catalogue/GetIngestHistory" */
    static const CORE::CString GetSavedSearches;            /**< "Catalogue/GetSavedSearches" */
    static const CORE::CString SaveSearch;                  /**< "Catalogue/SaveSearch" */
    static const CORE::CString DeleteSavedSearch;           /**< "Catalogue/DeleteSavedSearch" */
    static const CORE::CString RunSavedSearch;              /**< "Catalogue/RunSavedSearch" */
    static const CORE::CString GetAssetFileSet;             /**< "Catalogue/GetAssetFileSet" */
    static const CORE::CString GetLicence;                  /**< "Catalogue/GetLicence" */

    // Curation
    static const CORE::CString UpdatePackMetadata;          /**< "Curation/UpdatePackMetadata" */
    static const CORE::CString SetClassification;           /**< "Curation/SetClassification" */
    static const CORE::CString SetCategoryMapping;          /**< "Curation/SetCategoryMapping" */
    static const CORE::CString RemoveCategoryMapping;       /**< "Curation/RemoveCategoryMapping" */
    static const CORE::CString SetSynonym;                  /**< "Curation/SetSynonym" */
    static const CORE::CString SetCategoryRule;             /**< "Curation/SetCategoryRule" */
    static const CORE::CString RemoveCategoryRule;          /**< "Curation/RemoveCategoryRule" */
    static const CORE::CString CountRuleMatches;            /**< "Curation/CountRuleMatches" */
    static const CORE::CString Reclassify;                  /**< "Curation/Reclassify" */
    static const CORE::CString RemovePack;                  /**< "Curation/RemovePack" */

    // Thumbnails
    static const CORE::CString GetThumbnails;               /**< "Thumbnails/Get" */
    static const CORE::CString PutThumbnail;                /**< "Thumbnails/Put" */
    static const CORE::CString GetMissingThumbnails;        /**< "Thumbnails/GetMissing" */

    // Files
    static const CORE::CString StatFiles;                   /**< "Files/Stat" */
    static const CORE::CString ListDirectory;               /**< "Files/List" */
    static const CORE::CString ReadFileBlock;               /**< "Files/Read" */
    static const CORE::CString BeginUpload;                 /**< "Files/BeginUpload" */
    static const CORE::CString WriteUploadBlock;            /**< "Files/WriteUpload" */
    static const CORE::CString QueryUpload;                 /**< "Files/QueryUpload" */
    static const CORE::CString CommitUpload;                /**< "Files/CommitUpload" */
    static const CORE::CString AbortUpload;                 /**< "Files/AbortUpload" */

    // Locations
    static const CORE::CString GetLocations;                /**< "Locations/List" */
    static const CORE::CString RefreshLocationStates;       /**< "Locations/Refresh" */
    static const CORE::CString RelocateLocation;            /**< "Locations/Relocate" */
    static const CORE::CString VerifyLocation;              /**< "Locations/Verify" */

    // Ingest
    static const CORE::CString Ingest;                      /**< "Ingest/Ingest" */
    static const CORE::CString ResumeIngest;                /**< "Ingest/Resume" */
    static const CORE::CString GetPendingIngests;           /**< "Ingest/ListPending" */
    static const CORE::CString ListImportRoots;             /**< "Ingest/ListImportRoots" */
    static const CORE::CString SetSourcePolicy;             /**< "Ingest/SetSourcePolicy" */

    // Jobs
    static const CORE::CString ListJobs;                    /**< "Jobs/List" */
    static const CORE::CString GetJob;                      /**< "Jobs/Get" */
    static const CORE::CString ControlJob;                  /**< "Jobs/Control" */
    static const CORE::CString GetJobResult;                /**< "Jobs/GetResult" */

    // Stores
    static const CORE::CString ListStores;                  /**< "Stores/List" */
    static const CORE::CString QueryLibrary;                /**< "Stores/QueryLibrary" */
    static const CORE::CString GetStoreItem;                /**< "Stores/GetItem" */
    static const CORE::CString EnqueueDownload;             /**< "Stores/EnqueueDownload" */

    // SignIn
    static const CORE::CString BeginSignIn;                 /**< "SignIn/Begin" */
    static const CORE::CString AnswerPrompt;                /**< "SignIn/Answer" */
    static const CORE::CString CancelSignIn;                /**< "SignIn/Cancel" */
    static const CORE::CString SignOut;                     /**< "SignIn/SignOut" */
    static const CORE::CString GetSignInPrompts;            /**< "SignIn/ListPrompts" */

    // Conversion
    static const CORE::CString RequestConversion;           /**< "Conversion/Request" */
    static const CORE::CString RequestExport;               /**< "Conversion/Export" */

    // Pipeline
    static const CORE::CString RunPipelineQuery;            /**< "Pipeline/Query" */
    static const CORE::CString ApplyPipelineChange;         /**< "Pipeline/ApplyChange" */
    static const CORE::CString StartPipelineJob;            /**< "Pipeline/StartJob" */
    static const CORE::CString RenderSheet;                 /**< "Pipeline/RenderSheet" */

    // LocalCopy
    static const CORE::CString RequestLocalCopy;            /**< "LocalCopy/Request" */
    static const CORE::CString SetOfflinePin;               /**< "LocalCopy/SetOfflinePin" */

    // Admin
    static const CORE::CString UnlockKeyring;               /**< "Admin/UnlockKeyring" */
    static const CORE::CString LockKeyring;                 /**< "Admin/LockKeyring" */
    static const CORE::CString GetAuditRecords;             /**< "Admin/GetAuditRecords" */
    static const CORE::CString ReindexFromStorage;          /**< "Admin/ReindexFromStorage" */
    static const CORE::CString GetResourceStatus;           /**< "Admin/GetResourceStatus" */

    /** Whether the operation changes something and carries a command id */
    static bool IsMutation( const CORE::CString& operation );

    /** Whether the operation is one of this list */
    static bool IsKnown( const CORE::CString& operation );

    private:

    CAssetServiceOperations( void );    /**< not implemented: names only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */
