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

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORYWITHPARAM_H
#include "CTFactoryWithParam.h"
#define GUCEF_CORE_CTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H
#include "gucefASSETSTORE_CAssetStoreImageCache.h"
#define GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H
#include "gucefASSETSTORE_CIAssetStoreInteractiveSignIn.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETRENDERHOOK_H
#include "gucefASSETSTORE_CIAssetRenderHook.h"
#define GUCEF_ASSETSTORE_CIASSETRENDERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETRENDERHOOK_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#include "gucefASSETSTORE_CAssetLibrarySyncAdmission.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H ? */

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

class CLocalAssetStoreServiceOperationTaskConsumer;
class CSignInWakeup;
class CAssetLibrarySyncJob;

/**
 *  The asset store service in this process (section 4.4 of the service design): the primary store of the settings, the
 *  download manager, the listing picture cache and the online stores of the global registry
 *  (CAssetStoreGlobal::GetRegistry()), with the converter hook and the render hook of the configuration. No network
 *  component is involved: IsInProcess() is true and every operation runs the code of the store directly, as the viewer
 *  and the tools did before the service existed. A host that serves the store to remote clients runs this
 *  implementation behind it.
 *
 *  Open() makes and opens the primary store (a store that cannot be opened leaves the service open without its
 *  catalogue: the downloads and the online stores still work, GetStatusMessage() says why), attaches the download
 *  manager, reads its queue and starts it, and adopts the online stores of the registry. Close() undoes it after the
 *  asynchronous calls that run have ended.
 *
 *  The jobs of the service are the downloads of the manager (their ids), the ingests ("ingest:" and the ingest id), the
 *  conversions of packs that are in the store ("convert:" and a key of the service) and the jobs of the converter
 *  pipeline ("pipeline:" and a key). Ingests and conversions are known from the events of the store, also when another
 *  part of the process started them. A download for the primary store is ingested by the service when it is done, with
 *  the keep policy and location of its request, unless its source has the automatic policy (the store ingests those).
 *
 *  The change journal is a ring of the last 4096 changes of the catalogue with a revision that grows with every change;
 *  the outcomes of mutations are kept by command id (the last 1024) so a repeat answers the first outcome. The asynchronous
 *  forms run as tasks on the thread pool of the configuration; the ingests, their resumption and the verification of
 *  a location use the asynchronous forms of the store itself, so a cancellation of their future reaches them. Events are
 *  raised on the thread of the pulse generator of the configuration: directly for calls made on that thread, mailed to it
 *  otherwise. Thread safe.
 *
 *  Operations without an in-process implementation in this version answer ASSETSTORE_STATUS_UNSUPPORTED with the reason:
 *  thumbnails of the catalogue (the thumbnail store), uploads, exports, local copies, curation beyond mappings, synonyms and
 *  rules and the audit; the pipeline operations and renders answer what the converter hook and the render hook answer.
 *
 *  Sign in (the remote design, section 5.7): BeginSignIn() starts the sign in object of a store (CIAssetStoreProvider::
 *  GetInteractiveSignIn()), whose steps of a person come as SignInPromptEvent (an address to open, a code to enter) and are
 *  answered with AnswerPrompt(); the end of the sign in closes its prompts (SignInPromptClosedEvent) and changes the state of
 *  the store (StoreStateChangedEvent, CAssetStoreInfo::signInState). The sign in objects and the flows of gucefAUTH they use run
 *  on the thread that pumps the pulse generator of the service: a call from another thread waits for it (a call on that thread
 *  runs at once). A step that is not answered within CAssetStoreServiceConfig::signInPromptSeconds ends the sign in as failed.
 *  A store that signs in on its web site needs a browser beside the service, which this version does not have: its sign in answers
 *  BROWSER_UNAVAILABLE and the store says so in its state. The keyring (UnlockKeyring(), LockKeyring()) is the session of
 *  CAssetStoreSignInContext::keyringSession; its code is never kept or logged.
 *
 *  The pack index (pack index design, section 6): GetPackIndex(), GetResourceGroups() and GetResources() answer from the
 *  catalogue and the document of the pack; the job "index-pack" (StartPipelineJob() with that operation, of the kind
 *  "index-pack" in the list of jobs) loads or builds the index of one pack, of every pack without one (argument "missing") or
 *  of every pack from the documents on the storage (argument "fromStorage", which ReindexFromStorage() starts). The change
 *  ASSETCHANGE_PACK_INDEX_UPDATED follows every load that changed rows, after ASSETCHANGE_PACK_ADDED for an ingest.
 *
 *  The library sync (library sync design): the job "sync-library" (StartPipelineJob() with that operation and a
 *  CAssetLibrarySyncSpec in the arguments, of the kind "pipeline" and sub kind "sync" in the list of jobs) lists the whole
 *  library of a store, plans it against the primary store and, in the mode "run", downloads and ingests every format of
 *  every item that is missing or changed through EnqueueDownload(), a window of items at a time (CAssetLibrarySyncJob).
 *  It runs on the pulse generator of the service, not on the pool; one sync of a store at a time. CANCEL stops it and
 *  cancels its downloads that have not finished, RETRY of an ended sync plans again with the items that failed before.
 *  GetJobResult() gives the plan and the counts (section "plan") and the lines of the items in pages (section "items").
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CLocalAssetStoreService : public CIAssetStoreService
{
    public:

    static const CORE::CString IngestJobPrefix;         /**< "ingest:" */
    static const CORE::CString ConversionJobPrefix;     /**< "convert:" */
    static const CORE::CString PipelineJobPrefix;       /**< "pipeline:" */
    static const UInt32 JournalCapacity = 4096;
    static const UInt32 CommandCapacity = 1024;

    explicit CLocalAssetStoreService( const CAssetStoreServiceConfig& config );

    virtual ~CLocalAssetStoreService() GUCEF_VIRTUAL_OVERRIDE;

    /*
     *  What only the in-process implementation has: the parts of the process it works with
     */

    const CAssetStoreServiceConfig& GetConfig( void ) const;

    /** The objects of the store; GUCEF_NULL while the service is not open */
    CPrimaryAssetStore* GetPrimaryStore( void ) const;
    CAssetStoreDownloadManager* GetDownloadManager( void ) const;
    CAssetStoreImageCache* GetImageCache( void ) const;

    /*
     *  The library sync jobs (CAssetLibrarySyncJob) and the restart: a job reports what a restart needs to go on as a
     *  checkpoint, which the service keeps in its state file ("syncJobs") with the ledger of the caps per store
     *  ("syncLedgers"); its work list goes to a file of its own in the state folder. The start of the service restores
     *  the jobs of the checkpoints (ResumePendingWork()) under their ids. Without a state folder nothing is kept.
     */

    /** Keeps the checkpoint of a sync job and the ledger of its store; on the thread of the pulse generator */
    void NoteSyncCheckpoint( const CORE::CString& jobId, const CORE::CString& storeId, const CORE::CDataNode& checkpoint, const CAssetLibrarySyncAdmission::TLedgerVector& ledger );

    /** A sync job ended: its checkpoint and work list file go, the ledger of its store stays */
    void NoteSyncEnded( const CORE::CString& jobId, const CORE::CString& storeId, const CAssetLibrarySyncAdmission::TLedgerVector& ledger );

    /** The ledger of the caps of a store over its runs, empty when none was kept */
    CAssetLibrarySyncAdmission::TLedgerVector GetSyncLedger( const CORE::CString& storeId ) const;

    /** The VFS path of the work list file of a sync job, empty when the service keeps no state */
    CORE::CString GetSyncWorklistPath( const CORE::CString& jobId ) const;

    /*
     *  The schedule of the library sync (library sync design, section 6.4): a store whose profile is enabled and has a
     *  schedule (a cron expression in UTC) is a job "sync-library.<store>" of the job scheduler of the service, whose run
     *  starts the sync of the store with its profile (CAssetLibrarySyncScheduleTaskConsumer). No schedule by default. The end
     *  of a sync is a NoticeEvent (codes SYNC_DONE, SYNC_FAILED, SYNC_CANCELLED; SYNC_SIGN_IN_NEEDED while one waits).
     */

    /** The key under which the open service is found by the runs of its schedule; empty while it is not open */
    const CORE::CString& GetScheduleKey( void ) const;

    /** The job scheduler of the schedules of the library sync; null while no store has a schedule */
    CORE::JobSchedulerPtr GetSyncScheduler( void ) const;

    /** The id of the job of the schedule of a store */
    static CORE::CString MakeScheduleJobId( const CORE::CString& storeId );

    /** A run of a schedule: starts the scheduled sync of the store on the open service of the key */
    static TAssetStoreStatus StartScheduledSync( const CORE::CString& serviceKey, const CORE::CString& storeId, CORE::CString& jobId, CORE::CString& message );

    /** Another converter hook (not owned, must outlive the service); GUCEF_NULL takes the converter away */
    void SetConverterHook( CIAssetConverterHook* converterHook );
    CIAssetConverterHook* GetConverterHook( void ) const;

    /** Another render hook (not owned, must outlive the service) */
    void SetRenderHook( CIAssetRenderHook* renderHook );
    CIAssetRenderHook* GetRenderHook( void ) const;

    /** The revision of the catalogue: the number of changes the journal has seen */
    UInt64 GetCatalogueRevision( void ) const;

    /** The number of asynchronous calls that run */
    UInt32 GetPendingCallCount( void ) const;

    /*
     *  CIAssetStoreService
     */

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsInProcess( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsCatalogueAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetStatusMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ExecuteAsync( const CAssetServiceCall& call ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetStoreProvider( const CORE::CString& storeId, TAssetStoreProviderPtr& provider ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool TryGetListingImage( const CORE::CString& url, UInt32 size, CAssetThumbnail& image ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestListingImage( const CORE::CString& url, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void CancelListingImage( const CORE::CString& url, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    // Info

    virtual TAssetStoreStatus GetServiceInfo( CAssetServiceInfo& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetCatalogueChanges( UInt64 sinceRevision, UInt32 maxEntries, CAssetCatalogueChangePage& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Catalogue

    virtual TAssetStoreStatus Search( const CAssetSearchQuery& query, CAssetSearchResultPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus Suggest( const CORE::CString& prefix, UInt32 maxCount, CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetVocabulary( CAssetVocabularyInfo& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetPack( const CORE::CString& packId, CAssetPackRecord& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetAssets( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRecordPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetAsset( const CORE::CString& assetId, CAssetRecord& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetRelations( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRelationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetIngestHistory( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetIngestHistoryPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSavedSearches( const CORE::CString& pageToken, UInt32 maxEntries, CAssetSavedSearchPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SaveSearch( const CORE::CString& commandId, const CAssetSavedSearch& search, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus DeleteSavedSearch( const CORE::CString& commandId, const CORE::CString& searchId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetAssetFileSet( const CORE::CString& assetId, CAssetFileSet& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetLicence( const CORE::CString& licenceOrPackId, CAssetLicence& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetPackIndex( const CORE::CString& packId, bool forLearning, CAssetPackIndexInfo& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetResourceGroups( const CORE::CString& packId, bool forLearning, CAssetPackGroupPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, const CORE::CString& pageToken, UInt32 maxEntries, bool forLearning, CAssetPackResourcePage& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Curation

    virtual TAssetStoreStatus UpdatePackMetadata( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetClassification( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RemoveCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSynonym( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetCategoryRule( const CORE::CString& commandId, const CAssetCategoryRule& rule, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RemoveCategoryRule( const CORE::CString& commandId, const CORE::CString& ruleId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus CountRuleMatches( const CAssetCategoryRule& rule, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus Reclassify( const CORE::CString& commandId, const CORE::CString::StringVector& packIds, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RemovePack( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Thumbnails

    virtual TAssetStoreStatus GetThumbnails( const CORE::CString::StringVector& assetIds, UInt32 size, CAssetThumbnailPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetRestrictionsOfPath( const CORE::CString& vfsPath, CORE::CString::StringSet& flags ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus PutThumbnail( const CORE::CString& commandId, const CAssetThumbnail& thumbnail, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetMissingThumbnails( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries, CAssetThumbnailPage& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Files

    virtual TAssetStoreStatus StatFiles( const CORE::CString::StringVector& vfsPaths, CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ListDirectory( const CORE::CString& vfsPath, const CORE::CString& pageToken, UInt32 maxEntries, CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ReadFileBlock( const CORE::CString& vfsPath, UInt64 offset, UInt32 length, CAssetFileBlock& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus BeginUpload( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus QueryUpload( const CORE::CString& uploadId, CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus WriteUploadBlock( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult WriteUploadBlockAsync( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus CommitUpload( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus AbortUpload( const CORE::CString& commandId, const CORE::CString& uploadId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Locations

    virtual TAssetStoreStatus GetLocations( CAssetLocationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RefreshLocationStates( CAssetLocationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RelocateLocation( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus VerifyLocation( const CORE::CString& commandId, const CORE::CString& locationId, CAssetVerificationReport& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Ingest

    virtual TAssetStoreStatus Ingest( const CORE::CString& commandId, const CAssetIngestRequest& request, CAssetIngestResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ResumeIngest( const CORE::CString& commandId, const CORE::CString& ingestId, CAssetIngestResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetPendingIngests( CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ListImportRoots( CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSourcePolicy( const CORE::CString& commandId, const CAssetSourcePolicy& policy, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Jobs

    virtual TAssetStoreStatus ListJobs( const CORE::CString& pageToken, UInt32 maxEntries, CAssetJobPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetJob( const CORE::CString& jobId, CAssetJobEntry& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Stores

    virtual TAssetStoreStatus ListStores( CAssetStoreInfoPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus QueryLibrary( const CORE::CString& storeId, const CAssetStoreQuery& query, CAssetStoreLibraryPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetStoreItem( const CORE::CString& storeId, const CORE::CString& itemId, CAssetStoreItem& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus EnqueueDownload( const CORE::CString& commandId, const CAssetDownloadRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    // SignIn

    virtual TAssetStoreStatus BeginSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus CancelSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SignOut( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSignInPrompts( CAssetSignInPromptPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus AnswerPrompt( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult AnswerPromptAsync( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer ) GUCEF_VIRTUAL_OVERRIDE;

    // Conversion

    virtual TAssetStoreStatus RequestConversion( const CORE::CString& commandId, const CAssetConversionJobRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RequestExport( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Pipeline

    virtual TAssetStoreStatus RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;

    // LocalCopy

    virtual TAssetStoreStatus RequestLocalCopy( const CORE::CString& commandId, const CAssetLocalCopyRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetOfflinePin( const CORE::CString& commandId, const CORE::CString& packId, bool pinned, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    // Admin

    virtual TAssetStoreStatus LockKeyring( const CORE::CString& commandId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus UnlockKeyring( const CORE::CString& commandId, const AUTH::CSecretString& code, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult UnlockKeyringAsync( const CORE::CString& commandId, const AUTH::CSecretString& code ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetAuditRecords( const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ReindexFromStorage( const CORE::CString& commandId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetResourceStatus( CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSyncMarks( const CORE::CString& storeId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetSyncMarkPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSyncMarks( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetSyncMarkPage& marks, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSyncProfile( const CORE::CString& storeId, CAssetLibrarySyncProfile& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSyncProfile( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetLibrarySyncProfile& profile, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CLocalAssetStoreServiceOperationTaskConsumer;

    typedef CORE::CTEventHandlerFunctor< CLocalAssetStoreService > TEventCallback;

    struct SIngest
    {
        CORE::CString ingestId;
        CORE::CString packId;
        CORE::CString name;
        CORE::CString step;
        CORE::CString message;
        UInt32 stepIndex;
        TAssetJobState state;
        Int64 createdAtMs;
        Int64 updatedAtMs;
        SIngest( void );
    };
    typedef GUCEF::vector< SIngest > TIngestVector;

    struct SConversion
    {
        CORE::CString key;
        CORE::CString jobId;                     /**< the job id of the events of the store */
        CORE::CString packId;
        CORE::CString name;
        CORE::CString message;
        UInt32 done;
        UInt32 total;
        TAssetJobState state;
        TAssetStoreStatus status;
        CORE::CFutureResult future;
        Int64 createdAtMs;
        Int64 updatedAtMs;
        SConversion( void );
    };
    typedef GUCEF::vector< SConversion > TConversionVector;

    struct SPipelineJob
    {
        CORE::CString key;
        CAssetPipelineRequest request;
        CORE::CString name;
        CORE::CString message;
        UInt32 done;
        UInt32 total;
        TAssetJobState state;
        TAssetStoreStatus status;
        CORE::CString resultText;
        UInt32 failedCount;
        CAssetServiceDocument result;
        CORE::CFutureResultSource source;
        Int64 createdAtMs;
        Int64 updatedAtMs;
        SPipelineJob( void );
    };
    typedef GUCEF::vector< SPipelineJob > TPipelineJobVector;

    struct SRate
    {
        UInt64 bytesPerSecond;
        UInt64 secondsLeft;
    };
    typedef GUCEF::map< CORE::CString, SRate > TRateMap;

    struct SAddJob
    {
        CORE::CString jobId;
        CORE::CString keep;
        CORE::CString locationId;
        bool ingestRequested;
    };
    typedef GUCEF::vector< SAddJob > TAddJobVector;

    struct SCommand
    {
        CORE::CString operation;
        CAssetServiceResult outcome;
    };
    typedef GUCEF::map< CORE::CString, SCommand > TCommandMap;

    /** A call of the sign in that is carried out on the thread of the pulse generator */
    struct SSignInCommand
    {
        enum EKind
        {
            KIND_BEGIN = 0  ,
            KIND_ANSWER     ,
            KIND_CANCEL     ,
            KIND_SIGN_OUT
        };

        EKind kind;
        CORE::CString storeId;
        CORE::CString promptId;
        AUTH::CSecretString answer;
        TAssetStoreStatus status;
        CORE::CString message;
        CORE::CString id;
        MT::CTAtomic< UInt32 > done;
        MT::CTAtomic< UInt32 > abandoned;

        SSignInCommand( void );
    };
    typedef CORE::CTSharedPtr< SSignInCommand, MT::CMutex > TSignInCommandPtr;
    typedef GUCEF::vector< TSignInCommandPtr > TSignInCommandVector;

    /** A sign in that runs */
    struct SSignInRun
    {
        TAssetStoreProviderPtr provider;
        CORE::CString storeId;
        CORE::CString::StringVector promptIds;
        Int64 expiresMs;                        /**< UTC milliseconds; the step of the person that is not answered by then ends the sign in */
        UInt32 wrongAnswers;

        SSignInRun( void );
    };
    typedef GUCEF::map< CORE::CString, SSignInRun > TSignInRunMap;

    /** What the service knows of the sign in of a store besides what its provider says */
    struct SStoreSignInInfo
    {
        TAssetStoreSignInState state;           /**< of the last sign in that ended: failed, expired or signed out; signed in follows the provider */
        CORE::CString message;
        CORE::CString problem;                  /**< why the store cannot sign in here, from CIAssetStoreProvider::ConfigureSignIn() */
        bool wasSignedIn;

        SStoreSignInInfo( void );
    };
    typedef GUCEF::map< CORE::CString, SStoreSignInInfo > TStoreSignInInfoMap;

    // events of the parts
    void OnDownloadJobEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDownloadJobProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDownloadJobFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDownloadJobRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnIngestProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnIngestFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnIngestFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConversionProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConversionFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConversionFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLocationStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPackIndexUpdated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    TAssetStoreStatus RunIndexPackJob( const CAssetPipelineRequest& request, CIAssetConversionObserver* observer, const CORE::CFutureResultSource& source, CAssetServiceDocument& document );
    TAssetStoreStatus StartSyncJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result );
    void OnSyncJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    CAssetLibrarySyncJob* FindSyncJobLocked( const CORE::CString& key ) const;
    /** The profile kept for the store, the defaults when none was set */
    CAssetLibrarySyncProfile GetStoredSyncProfile( const CORE::CString& storeId ) const;
    /** Makes the sync jobs of the checkpoints of the state file again, under their ids; they go on where they were */
    void RestoreSyncJobs( void );
    /** Brings the jobs of the scheduler in line with the profiles: a job per enabled profile with a schedule */
    void UpdateSyncSchedules( void );
    void StopSyncSchedules( void );
    /** The notice of a sync job whose state changed (its end, or the wait for a sign in) */
    void NoteSyncNotice( const CORE::CString& jobId, const CORE::CString& storeId, TAssetJobState state, TAssetStoreStatus status, const CORE::CString& text );
    void OnProviderAdded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProviderRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnWebSessionEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignInWake( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignInTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnImageReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    // the jobs
    void NoteIngest( CORE::CICloneable* eventData, TAssetJobState state );
    void NoteConversion( CORE::CICloneable* eventData, TAssetJobState state );
    SConversion* FindConversionOfEventLocked( const CORE::CString& jobId, const CORE::CString& packId );
    TAssetStoreStatus StartConversionJob( const CORE::CString& packId, CORE::CString& jobId );
    void IngestFinishedDownload( const CAssetStoreDownloadJob& job, bool atStart = false );

    /** The state folder of the settings: mounted in the VFS, true when the service keeps its restart state */
    bool MountStateDirectory( void );
    void UnmountStateDirectory( void );

    /** The thumbnails folder of the settings: mounted in the VFS, true when the pictures are kept there instead of in the storage location */
    bool MountThumbnailsDirectory( void );
    void UnmountThumbnailsDirectory( void );

    /** Reads the state file: the policies set while the service ran, the downloads that are to be ingested and the resumes that were tried */
    void LoadServiceState( void );

    /** Writes the state file (when the service keeps one); called after each change of what it holds */
    void SaveServiceState( void );

    /** After a start: the downloads that finished with no ingest requested are ingested, the staged ingests resume */
    void ResumePendingWork( void );
    void RunPipelineJob( const CORE::CString& key );

    /** Starts the request of an ended pipeline job again as a job of its own (what the cache holds is kept); with failedOnly, only the assets that failed. Answers the status and the id of the new job */
    TAssetStoreStatus RestartPipelineJob( const CORE::CString& key, bool failedOnly, CORE::CString& newJobId, CORE::CString& problem );
    void NotePipelineProgress( const CORE::CString& key, UInt32 done, UInt32 total, const CORE::CString& assetPath );
    void MakeDownloadEntry( const CAssetStoreDownloadJob& job, CAssetJobEntry& entry ) const;
    void MakeIngestEntry( const SIngest& ingest, CAssetJobEntry& entry ) const;
    void MakeConversionEntry( const SConversion& conversion, CAssetJobEntry& entry ) const;
    void MakePipelineEntry( const SPipelineJob& job, CAssetJobEntry& entry ) const;
    void GetAllJobs( TAssetJobEntryVector& jobs ) const;
    bool FindJob( const CORE::CString& jobId, CAssetJobEntry& entry ) const;
    void NotifyJobChanged( const CORE::CString& jobId );
    void NotifyJobRemoved( const CAssetJobEntry& entry );
    CORE::CString GetPackName( const CORE::CString& packId ) const;

    // the stores
    void WatchProviders( void );
    void MakeStoreInfo( const TAssetStoreProviderPtr& provider, CAssetStoreInfo& info ) const;
    void NotifyStoreState( const CORE::CString& storeId );
    void ConfigureProviders( void );
    // the sign in
    TAssetStoreStatus RunSignInCommand( SSignInCommand::EKind kind, const CORE::CString& storeId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result );
    void ExecuteSignInCommand( SSignInCommand& command );
    void ProcessSignInCommands( void );
    void BeginSignInOnThread( SSignInCommand& command );
    void AnswerPromptOnThread( SSignInCommand& command );
    void CancelSignInOnThread( SSignInCommand& command );
    void SignOutOnThread( SSignInCommand& command );
    void PollSignInRuns( void );
    void EndSignInRun( const CORE::CString& storeId, TAssetStoreSignInState state, const CORE::CString& message );
    void ClosePrompt( const CORE::CString& promptId );
    void NoteStoreStatus( const CORE::CString& storeId, TAssetStoreStatus status );
    void UpdateSignInTimer( void );
    CORE::CString GetKeyringState( void ) const;

    // the journal, the commands and the events
    void RecordChange( TAssetCatalogueChangeKind kind, const CORE::CString& id );
    bool ReplayCommand( const CORE::CString& commandId, const CORE::CString& operation, CAssetServiceResult& result );
    void RememberCommand( const CORE::CString& commandId, const CORE::CString& operation, const CAssetServiceResult& result );
    void Raise( const CORE::CEvent& eventId, const CORE::CICloneable& eventData );
    bool IsOnNotificationThread( void ) const;
    TAssetStoreStatus Refuse( TAssetStoreStatus status, const CORE::CString& message, CAssetServiceResult& result ) const;
    TAssetStoreStatus NeedCatalogue( CORE::CString& message ) const;
    CORE::CFutureResult QueueCall( const CAssetServiceCall& call, const CORE::CString& pipelineJobKey );

    // the stored thumbnails and the licence gate of paths
    CORE::CString GetThumbnailFolder( void ) const;
    bool GetThumbnailDigest( const CORE::CString& id, CORE::CString& digest, CORE::CString& message ) const;

    struct SPathRestriction
    {
        bool inPack;
        CORE::CString::StringSet flags;
        UInt64 revision;
        SPathRestriction( void ) : inPack( false ), flags(), revision( 0 ) {}
    };
    typedef GUCEF::map< CORE::CString, SPathRestriction > TPathRestrictionMap;
    void CallEnded( void );
    void GetLocationInfos( TAssetLocationInfoVector& locations ) const;

    CLocalAssetStoreService( const CLocalAssetStoreService& src );              /**< not implemented */
    CLocalAssetStoreService& operator=( const CLocalAssetStoreService& src );   /**< not implemented */

    private:

    CAssetStoreServiceConfig m_config;
    TPrimaryAssetStorePtr m_storePtr;
    CPrimaryAssetStore* m_store;
    CAssetStoreDownloadManager* m_manager;
    CAssetStoreImageCache* m_imageCache;
    bool m_open;
    bool m_catalogueOpen;
    CORE::CString m_statusMessage;
    TIngestVector m_ingests;
    TConversionVector m_conversions;
    TPipelineJobVector m_pipelineJobs;
    GUCEF::map< CORE::CString, CAssetLibrarySyncJob* > m_syncJobs;    /**< the library syncs by the key of their pipeline job; owned */
    GUCEF::map< CORE::CString, CAssetLibrarySyncProfile > m_syncProfiles;   /**< the sync profiles of the stores by store id, kept in the state file */
    GUCEF::map< CORE::CString, CORE::CDataNode > m_syncCheckpoints;         /**< the checkpoints of the sync jobs that did not end, by job key, kept in the state file */
    GUCEF::map< CORE::CString, CAssetLibrarySyncAdmission::TLedgerVector > m_syncLedgers;  /**< the ledgers of the caps by store id, kept in the state file */
    CORE::CString m_scheduleKey;
    CORE::JobSchedulerPtr m_syncScheduler;
    GUCEF::map< CORE::CString, CORE::CString > m_syncSchedules;          /**< the stores with a schedule job and its cron expression */
    CORE::CString::StringSet m_syncNoticesRaised;                       /**< "<jobId>|<code>" of the notices raised, once each */
    CORE::CFutureResult m_pendingConversions;
    TRateMap m_rates;
    TAddJobVector m_addJobs;
    bool m_stateKept;
    bool m_thumbnailsMounted;
    GUCEF::vector< CAssetSourcePolicy > m_runtimePolicies;     /**< the policies set through the service, kept in the state file */
    GUCEF::map< CORE::CString, UInt32 > m_ingestResumes;       /**< per staged ingest: the starts that resumed it, kept in the state file */
    MT::CMutex m_stateLock;
    UInt32 m_conversionCounter;
    UInt32 m_pipelineCounter;
    CORE::CString::StringSet m_endedSessions;
    CORE::CString::StringSet m_watchedProviders;
    TAssetCatalogueChangeVector m_journal;
    UInt32 m_journalHead;
    UInt64 m_revision;
    TCommandMap m_commands;
    CORE::CString::StringVector m_commandOrder;
    TThreadId m_ownerThread;
    MT::CTAtomic< UInt32 > m_pendingCalls;
    TSignInRunMap m_signInRuns;
    TStoreSignInInfoMap m_signInInfos;
    TAssetSignInPromptVector m_prompts;
    TSignInCommandVector m_signInQueue;
    CORE::CString::StringSet m_configuredProviders;
    UInt32 m_promptCounter;
    CSignInWakeup* m_signInWakeup;
    CORE::CTimer* m_signInTimer;
    mutable MT::CMutex m_lock;
    mutable MT::CMutex m_restrictionLock;
    mutable TPathRestrictionMap m_pathRestrictions;
};

/** The factory of the in-process implementation, registered under "local" by CAssetStoreGlobal */
typedef CORE::CTFactoryWithParam< CIAssetStoreService, CLocalAssetStoreService, CAssetStoreServiceConfig, MT::CMutex > TLocalAssetStoreServiceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */
