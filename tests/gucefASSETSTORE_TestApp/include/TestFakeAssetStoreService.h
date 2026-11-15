/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTFAKEASSETSTORESERVICE_H
#define TESTFAKEASSETSTORESERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An asset store service for the tests of the code that works on CIAssetStoreService: every call is recorded (as the
 *  CAssetServiceCall the asynchronous form makes, so the arguments can be checked; secrets are left out), and answered
 *  with what the test scripted for the operation with SetAnswer(): a status and a result object (an operation without
 *  an answer is UNSUPPORTED). It can say that it is not in process, so the paths of adapters for a remote service are
 *  tested, hold the futures of the asynchronous forms until CompletePendingCalls() (SetDeferAsync()), and raise the
 *  events of the interface with Raise(). The events are raised on the thread that calls Raise().
 */
class CFakeAssetStoreService : public GUCEF::ASSETSTORE::CIAssetStoreService
{
    public:

    explicit CFakeAssetStoreService( GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::PulseGeneratorPtr() );

    virtual ~CFakeAssetStoreService();

    /** The answer of an operation: the status, and a copy of the result (GUCEF_NULL for none) */
    void SetAnswer( const GUCEF::CORE::CString& operation, GUCEF::ASSETSTORE::TAssetStoreStatus status, const GUCEF::CORE::CICloneable* result = GUCEF_NULL );
    void ClearAnswers( void );

    void SetInProcess( bool inProcess );
    void SetOpen( bool open );
    void SetCatalogueAvailable( bool available );
    void SetStatusMessage( const GUCEF::CORE::CString& message );

    /** Whether the futures of the asynchronous forms wait for CompletePendingCalls(); false (answered at once) by default */
    void SetDeferAsync( bool defer );

    /** Answers the asynchronous calls that wait; the number answered */
    GUCEF::UInt32 CompletePendingCalls( void );
    GUCEF::UInt32 GetPendingCallCount( void ) const;

    void SetStoreProvider( const GUCEF::CORE::CString& storeId, const GUCEF::ASSETSTORE::TAssetStoreProviderPtr& provider );

    /** A listing picture the service has: TryGetListingImage() gives it, RequestListingImage() raises ListingImageReadyEvent with it */
    void SetListingImage( const GUCEF::ASSETSTORE::CAssetThumbnail& image );
    GUCEF::UInt32 GetListingImageRequestCount( void ) const;

    /** Raises an event of the interface on the calling thread */
    void Raise( const GUCEF::CORE::CEvent& eventId );
    void Raise( const GUCEF::CORE::CEvent& eventId, const GUCEF::CORE::CICloneable& eventData );

    /** What was called, in order */
    GUCEF::UInt32 GetCallCount( void ) const;
    GUCEF::UInt32 CountCalls( const GUCEF::CORE::CString& operation ) const;
    bool GetLastCall( const GUCEF::CORE::CString& operation, GUCEF::ASSETSTORE::CAssetServiceCall& call ) const;
    void ClearCalls( void );

    virtual GUCEF::CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsInProcess( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsCatalogueAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetStatusMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CFutureResult ExecuteAsync( const GUCEF::ASSETSTORE::CAssetServiceCall& call ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetStoreProvider( const GUCEF::CORE::CString& storeId, GUCEF::ASSETSTORE::TAssetStoreProviderPtr& provider ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool TryGetListingImage( const GUCEF::CORE::CString& url, GUCEF::UInt32 size, GUCEF::ASSETSTORE::CAssetThumbnail& image ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestListingImage( const GUCEF::CORE::CString& url, GUCEF::UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void CancelListingImage( const GUCEF::CORE::CString& url, GUCEF::UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetServiceInfo( GUCEF::ASSETSTORE::CAssetServiceInfo& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetCatalogueChanges( GUCEF::UInt64 sinceRevision, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetCatalogueChangePage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus Search( const GUCEF::ASSETSTORE::CAssetSearchQuery& query, GUCEF::ASSETSTORE::CAssetSearchResultPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus Suggest( const GUCEF::CORE::CString& prefix, GUCEF::UInt32 maxCount, GUCEF::ASSETSTORE::CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetVocabulary( GUCEF::ASSETSTORE::CAssetVocabularyInfo& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetPack( const GUCEF::CORE::CString& packId, GUCEF::ASSETSTORE::CAssetPackRecord& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetAssets( const GUCEF::CORE::CString& packId, const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetRecordPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetAsset( const GUCEF::CORE::CString& assetId, GUCEF::ASSETSTORE::CAssetRecord& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetRelations( const GUCEF::CORE::CString& assetId, const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetRelationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetIngestHistory( const GUCEF::CORE::CString& packId, const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetIngestHistoryPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetSavedSearches( const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetSavedSearchPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SaveSearch( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetSavedSearch& search, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus DeleteSavedSearch( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& searchId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RunSavedSearch( const GUCEF::CORE::CString& searchId, GUCEF::UInt32 pageIndex, GUCEF::ASSETSTORE::CAssetSearchResultPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetAssetFileSet( const GUCEF::CORE::CString& assetId, GUCEF::ASSETSTORE::CAssetFileSet& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetLicence( const GUCEF::CORE::CString& licenceOrPackId, GUCEF::ASSETSTORE::CAssetLicence& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus UpdatePackMetadata( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetPackRecord& pack, GUCEF::UInt64 expectedRevision, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SetClassification( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& objectId, const GUCEF::ASSETSTORE::CAssetClassification& classification, const GUCEF::CORE::CString& source, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SetCategoryMapping( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& sourceId, const GUCEF::CORE::CString& storeCategory, const GUCEF::CORE::CString& categoryId, const GUCEF::CORE::CString& theme, const GUCEF::CORE::CString& kind, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RemoveCategoryMapping( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& sourceId, const GUCEF::CORE::CString& storeCategory, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SetSynonym( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& word, const GUCEF::CORE::CString& replacement, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SetCategoryRule( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetCategoryRule& rule, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RemoveCategoryRule( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& ruleId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus CountRuleMatches( const GUCEF::ASSETSTORE::CAssetCategoryRule& rule, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus Reclassify( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString::StringVector& packIds, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RemovePack( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& packId, bool deleteFiles, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetThumbnails( const GUCEF::CORE::CString::StringVector& assetIds, GUCEF::UInt32 size, GUCEF::ASSETSTORE::CAssetThumbnailPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus PutThumbnail( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetThumbnail& thumbnail, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetMissingThumbnails( const GUCEF::CORE::CString& rendererId, const GUCEF::CORE::CString& rendererVersion, GUCEF::UInt32 size, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetThumbnailPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus StatFiles( const GUCEF::CORE::CString::StringVector& vfsPaths, GUCEF::ASSETSTORE::CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ListDirectory( const GUCEF::CORE::CString& vfsPath, const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ReadFileBlock( const GUCEF::CORE::CString& vfsPath, GUCEF::UInt64 offset, GUCEF::UInt32 length, GUCEF::ASSETSTORE::CAssetFileBlock& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus BeginUpload( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& name, GUCEF::UInt64 totalBytes, GUCEF::UInt32 fileCount, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus QueryUpload( const GUCEF::CORE::CString& uploadId, GUCEF::ASSETSTORE::CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus CommitUpload( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& uploadId, const GUCEF::ASSETSTORE::CAssetIngestRequest& ingest, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus AbortUpload( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& uploadId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetLocations( GUCEF::ASSETSTORE::CAssetLocationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RefreshLocationStates( GUCEF::ASSETSTORE::CAssetLocationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RelocateLocation( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& locationId, const GUCEF::CORE::CString& newVfsRoot, const GUCEF::CORE::CString& newFileSystemPath, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus VerifyLocation( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& locationId, GUCEF::ASSETSTORE::CAssetVerificationReport& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus Ingest( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetIngestRequest& request, GUCEF::ASSETSTORE::CAssetIngestResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ResumeIngest( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& ingestId, GUCEF::ASSETSTORE::CAssetIngestResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetPendingIngests( GUCEF::ASSETSTORE::CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ListImportRoots( GUCEF::ASSETSTORE::CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SetSourcePolicy( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetSourcePolicy& policy, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ListJobs( const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetJobPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetJob( const GUCEF::CORE::CString& jobId, GUCEF::ASSETSTORE::CAssetJobEntry& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ControlJob( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& jobId, GUCEF::ASSETSTORE::TAssetJobAction action, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetJobResult( const GUCEF::CORE::CString& jobId, const GUCEF::CORE::CString& section, const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ListStores( GUCEF::ASSETSTORE::CAssetStoreInfoPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus QueryLibrary( const GUCEF::CORE::CString& storeId, const GUCEF::ASSETSTORE::CAssetStoreQuery& query, GUCEF::ASSETSTORE::CAssetStoreLibraryPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetStoreItem( const GUCEF::CORE::CString& storeId, const GUCEF::CORE::CString& itemId, GUCEF::ASSETSTORE::CAssetStoreItem& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus EnqueueDownload( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetDownloadRequest& request, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus BeginSignIn( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& storeId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus CancelSignIn( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& storeId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SignOut( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& storeId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetSignInPrompts( GUCEF::ASSETSTORE::CAssetSignInPromptPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RequestConversion( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetConversionJobRequest& request, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RequestExport( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString::StringVector& assetIds, const GUCEF::CORE::CString& profile, const GUCEF::CORE::CString& format, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RunPipelineQuery( const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ApplyPipelineChange( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus StartPipelineJob( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RenderSheet( const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RequestLocalCopy( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetLocalCopyRequest& request, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus SetOfflinePin( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& packId, bool pinned, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus LockKeyring( const GUCEF::CORE::CString& commandId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetAuditRecords( const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ReindexFromStorage( const GUCEF::CORE::CString& commandId, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetResourceStatus( GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus AnswerPrompt( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& promptId, const GUCEF::AUTH::CSecretString& answer, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CFutureResult AnswerPromptAsync( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& promptId, const GUCEF::AUTH::CSecretString& answer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus WriteUploadBlock( const GUCEF::CORE::CString& uploadId, const GUCEF::CORE::CString& relativePath, GUCEF::UInt64 offset, const GUCEF::CORE::CDynamicBuffer& data, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CFutureResult WriteUploadBlockAsync( const GUCEF::CORE::CString& uploadId, const GUCEF::CORE::CString& relativePath, GUCEF::UInt64 offset, const GUCEF::CORE::CDynamicBuffer& data ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus UnlockKeyring( const GUCEF::CORE::CString& commandId, const GUCEF::AUTH::CSecretString& code, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CFutureResult UnlockKeyringAsync( const GUCEF::CORE::CString& commandId, const GUCEF::AUTH::CSecretString& code ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SAnswer
    {
        GUCEF::ASSETSTORE::TAssetStoreStatus status;
        GUCEF::CORE::CICloneable* result;
    };
    typedef GUCEF::map< GUCEF::CORE::CString, SAnswer > TAnswerMap;

    struct SPending
    {
        GUCEF::ASSETSTORE::CAssetServiceCall call;
        GUCEF::CORE::CFutureResultSource source;
    };
    typedef GUCEF::vector< SPending > TPendingVector;

    /** Records the call and gives the scripted answer */
    template < class T >
    GUCEF::ASSETSTORE::TAssetStoreStatus Answer( const GUCEF::ASSETSTORE::CAssetServiceCall& call, T& result )
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        m_calls.push_back( call );
        TAnswerMap::const_iterator i = m_answers.find( call.operation );
        if ( i == m_answers.end() )
            return GUCEF::ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED;
        const T* scripted = GUCEF::CORE::CICloneable::CloneableAs< T >( (*i).second.result );
        if ( GUCEF_NULL != scripted )
            result = *scripted;
        return (*i).second.status;
    }

    CFakeAssetStoreService( const CFakeAssetStoreService& src );
    CFakeAssetStoreService& operator=( const CFakeAssetStoreService& src );

    private:

    TAnswerMap m_answers;
    GUCEF::vector< GUCEF::ASSETSTORE::CAssetServiceCall > m_calls;
    TPendingVector m_pending;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::ASSETSTORE::TAssetStoreProviderPtr > m_providers;
    GUCEF::vector< GUCEF::ASSETSTORE::CAssetThumbnail > m_images;
    GUCEF::UInt32 m_imageRequests;
    bool m_inProcess;
    bool m_open;
    bool m_catalogue;
    bool m_defer;
    GUCEF::CORE::CString m_statusMessage;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKEASSETSTORESERVICE_H ? */
