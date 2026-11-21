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

#ifndef GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H
#define GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CTASSETSERVICEPAGE_H
#include "gucefASSETSTORE_CTAssetServicePage.h"
#define GUCEF_ASSETSTORE_CTASSETSERVICEPAGE_H
#endif /* GUCEF_ASSETSTORE_CTASSETSERVICEPAGE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H
#include "gucefASSETSTORE_CAssetRelationRecord.h"
#define GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETINGESTHISTORYENTRY_H
#include "gucefASSETSTORE_CAssetIngestHistoryEntry.h"
#define GUCEF_ASSETSTORE_CASSETINGESTHISTORYENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTHISTORYENTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H
#include "gucefASSETSTORE_CAssetSavedSearch.h"
#define GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H
#endif /* GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#include "gucefASSETSTORE_CAssetJobEntry.h"
#define GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETJOBENTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLOCATIONINFO_H
#include "gucefASSETSTORE_CAssetLocationInfo.h"
#define GUCEF_ASSETSTORE_CASSETLOCATIONINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETLOCATIONINFO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREINFO_H
#include "gucefASSETSTORE_CAssetStoreInfo.h"
#define GUCEF_ASSETSTORE_CASSETSTOREINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREINFO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#include "gucefASSETSTORE_CAssetThumbnail.h"
#define GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUECHANGE_H
#include "gucefASSETSTORE_CAssetCatalogueChange.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUECHANGE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUECHANGE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETFILERECORD_H
#include "gucefASSETSTORE_CAssetFileRecord.h"
#define GUCEF_ASSETSTORE_CASSETFILERECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETFILERECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSIGNINPROMPT_H
#include "gucefASSETSTORE_CAssetSignInPrompt.h"
#define GUCEF_ASSETSTORE_CASSETSIGNINPROMPT_H
#endif /* GUCEF_ASSETSTORE_CASSETSIGNINPROMPT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKGROUP_H
#include "gucefASSETSTORE_CAssetPackGroup.h"
#define GUCEF_ASSETSTORE_CASSETPACKGROUP_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKGROUP_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H
#include "gucefASSETSTORE_CAssetPackResource.h"
#define GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARK_H
#include "gucefASSETSTORE_CAssetSyncMark.h"
#define GUCEF_ASSETSTORE_CASSETSYNCMARK_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The page types of the lists of the asset store service (CTAssetServicePage): each with the tag that names its cloneable type
 */

/** A page of records of assets */
struct SAssetRecordPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetRecordPage"; } };
typedef CTAssetServicePage< CAssetRecord, SAssetRecordPageName > CAssetRecordPage;

/** A page of relations of an asset */
struct SAssetRelationPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetRelationPage"; } };
typedef CTAssetServicePage< CAssetRelationRecord, SAssetRelationPageName > CAssetRelationPage;

/** A page of entries of the ingest history */
struct SAssetIngestHistoryPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetIngestHistoryPage"; } };
typedef CTAssetServicePage< CAssetIngestHistoryEntry, SAssetIngestHistoryPageName > CAssetIngestHistoryPage;

/** A page of saved searches */
struct SAssetSavedSearchPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetSavedSearchPage"; } };
typedef CTAssetServicePage< CAssetSavedSearch, SAssetSavedSearchPageName > CAssetSavedSearchPage;

/** A page of jobs */
struct SAssetJobPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetJobPage"; } };
typedef CTAssetServicePage< CAssetJobEntry, SAssetJobPageName > CAssetJobPage;

/** A page of storage locations with their state */
struct SAssetLocationPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetLocationPage"; } };
typedef CTAssetServicePage< CAssetLocationInfo, SAssetLocationPageName > CAssetLocationPage;

/** A page of online stores */
struct SAssetStoreInfoPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetStoreInfoPage"; } };
typedef CTAssetServicePage< CAssetStoreInfo, SAssetStoreInfoPageName > CAssetStoreInfoPage;

/** A page of thumbnails, listing pictures or thumbnail work items */
struct SAssetThumbnailPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetThumbnailPage"; } };
typedef CTAssetServicePage< CAssetThumbnail, SAssetThumbnailPageName > CAssetThumbnailPage;

/** A page of entries of the change journal of the catalogue */
struct SAssetCatalogueChangePageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetCatalogueChangePage"; } };
typedef CTAssetServicePage< CAssetCatalogueChange, SAssetCatalogueChangePageName > CAssetCatalogueChangePage;

/** A page of the group summaries of a pack index: the function groups and their sub-groups with their counts */
struct SAssetPackGroupPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetPackGroupPage"; } };
typedef CTAssetServicePage< CAssetPackGroup, SAssetPackGroupPageName > CAssetPackGroupPage;

/** A page of the resources of a pack index */
struct SAssetPackResourcePageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetPackResourcePage"; } };
typedef CTAssetServicePage< CAssetPackResource, SAssetPackResourcePageName > CAssetPackResourcePage;

/** A page of files of the service: VFS path, size, SHA-256 digest and the role "file", "directory" or "missing" */
struct SAssetFileStatPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetFileStatPage"; } };
typedef CTAssetServicePage< CAssetFileRecord, SAssetFileStatPageName > CAssetFileStatPage;

/** A page of open sign in prompts */
struct SAssetSignInPromptPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetSignInPromptPage"; } };
typedef CTAssetServicePage< CAssetSignInPrompt, SAssetSignInPromptPageName > CAssetSignInPromptPage;

/** A page of the marks of the owner on items of a store (library sync): ignore or pin; an entry without a mark removes one in SetSyncMarks() */
struct SAssetSyncMarkPageName { static const char* Name( void ) { return "GUCEF::ASSETSTORE::CAssetSyncMarkPage"; } };
typedef CTAssetServicePage< CAssetSyncMark, SAssetSyncMarkPageName > CAssetSyncMarkPage;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H ? */
