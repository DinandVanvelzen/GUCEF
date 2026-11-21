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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#define GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTFACTORYWITHPARAM_H
#include "CTFactoryWithParam.h"
#define GUCEF_CORE_CTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#include "gucefDRNUTILS_CRemoteServiceConnection.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICESETTINGS_H
#include "gucefASSETSTORE_CRemoteAssetStoreServiceSettings.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICESETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CRemoteAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTOREPROVIDER_H ? */

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
 *  The asset store service of a server (CAssetStoreServiceHost) through the network: the implementation "remote" of
 *  CIAssetStoreService (gucefASSETSTORE_RemoteService_Design.md 4.5). It overrides the one ExecuteAsync() of the
 *  interface: every asynchronous form becomes a generic call of the contracts of CAssetStoreRemoteContracts over a
 *  DRNUTILS::CRemoteServiceConnection (a control link and a bulk link of its own for the files; reconnects, resumes and
 *  backs off by itself). The synchronous forms wait for the asynchronous ones: they block on the network and must not
 *  be used on the thread of a GUI (IsInProcess() is false, so the viewer's adapters take their asynchronous paths).
 *
 *  - Answers come with the event sequence of the session after the call; a future completes once the session events
 *    before it were raised (at most eventWaitMs), so the events a call caused come before its result as in process.
 *  - Failures end the futures with "status: message" as in process; the synchronous forms give a record with both.
 *  - Events of the session become the events of the interface: catalogue changes, jobs, locations, stores, prompts,
 *    thumbnails, notices; a resync or a new session raises CatalogueChangedEvent with RESYNC (drop cached metadata);
 *    the connection's state raises ConnectionStateChangedEvent (CAssetServiceNotice, code: the state).
 *  - Files: WriteUploadBlock() writes in blocks of CAssetStoreRemoteContracts::BlockSize in parallel on the bulk link;
 *    BeginUploadFiles() announces an upload with its digests. The files of the store are read through the share of the
 *    service (CRemoteAssetShares).
 *  - Online stores: GetStoreProvider() gives a CRemoteAssetStoreProvider whose library calls go to the service.
 *    Listing pictures are not offered in this version (TryGetListingImage() and RequestListingImage() answer false).
 *  Threading: events and the completion of futures on the notification pulse generator of the configuration (a timer
 *  checks the calls every 10 ms); a synchronous form on the thread that drives it pumps it while it waits.
 *  Device credentials come from a credential store (see CRemoteAssetStoreServiceSettings); they are never logged.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteAssetStoreService : public CIAssetStoreService
{
    public:

    explicit CRemoteAssetStoreService( const CAssetStoreServiceConfig& config );

    virtual ~CRemoteAssetStoreService() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Pairs this device with a service through its pairing listener with the code of a pairing window, and keeps the
     *  credentials in the store under the slot. Blocking. The code is never logged. False with the reason
     */
    static bool PairDevice( const CORE::CString& pairingEndpoint, const AUTH::CSecretString& code, const CORE::CString& serviceName,
                            const CORE::CString& drnSessionName, const CORE::CString& deviceName, const CORE::CString& clientKind,
                            AUTH::ICredentialStore& store, const CORE::CString& slot, CORE::CString& error );

    /** The settings read from the configuration */
    const CRemoteAssetStoreServiceSettings& GetSettings( void ) const;

    /** The connection while open, NULL otherwise (tests, telemetry) */
    DRNUTILS::CRemoteServiceConnection* GetConnection( void ) const;

    /** Calls that wait for their answer */
    UInt32 GetPendingCallCount( void ) const;

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

    virtual TAssetStoreStatus GetServiceInfo( CAssetServiceInfo& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetCatalogueChanges( UInt64 sinceRevision, UInt32 maxEntries, CAssetCatalogueChangePage& result ) GUCEF_VIRTUAL_OVERRIDE;
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
    virtual TAssetStoreStatus GetThumbnails( const CORE::CString::StringVector& assetIds, UInt32 size, CAssetThumbnailPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus PutThumbnail( const CORE::CString& commandId, const CAssetThumbnail& thumbnail, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetMissingThumbnails( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries, CAssetThumbnailPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus BeginUpload( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus QueryUpload( const CORE::CString& uploadId, CAssetFileStatPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus CommitUpload( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus AbortUpload( const CORE::CString& commandId, const CORE::CString& uploadId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetLocations( CAssetLocationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RefreshLocationStates( CAssetLocationPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RelocateLocation( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus VerifyLocation( const CORE::CString& commandId, const CORE::CString& locationId, CAssetVerificationReport& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus Ingest( const CORE::CString& commandId, const CAssetIngestRequest& request, CAssetIngestResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ResumeIngest( const CORE::CString& commandId, const CORE::CString& ingestId, CAssetIngestResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetPendingIngests( CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ListImportRoots( CAssetServiceStringList& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSourcePolicy( const CORE::CString& commandId, const CAssetSourcePolicy& policy, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ListJobs( const CORE::CString& pageToken, UInt32 maxEntries, CAssetJobPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetJob( const CORE::CString& jobId, CAssetJobEntry& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ListStores( CAssetStoreInfoPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus QueryLibrary( const CORE::CString& storeId, const CAssetStoreQuery& query, CAssetStoreLibraryPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetStoreItem( const CORE::CString& storeId, const CORE::CString& itemId, CAssetStoreItem& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus EnqueueDownload( const CORE::CString& commandId, const CAssetDownloadRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus BeginSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus CancelSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SignOut( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSignInPrompts( CAssetSignInPromptPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RequestConversion( const CORE::CString& commandId, const CAssetConversionJobRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RequestExport( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus RequestLocalCopy( const CORE::CString& commandId, const CAssetLocalCopyRequest& request, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetOfflinePin( const CORE::CString& commandId, const CORE::CString& packId, bool pinned, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus LockKeyring( const CORE::CString& commandId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetAuditRecords( const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ReindexFromStorage( const CORE::CString& commandId, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetResourceStatus( CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSyncMarks( const CORE::CString& storeId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetSyncMarkPage& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSyncMarks( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetSyncMarkPage& marks, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetSyncProfile( const CORE::CString& storeId, CAssetLibrarySyncProfile& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSyncProfile( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetLibrarySyncProfile& profile, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus DescribeSettings( const CAssetSettingsRequest& request, CAssetSettingsDescription& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus SetSettings( const CORE::CString& commandId, const CAssetSettingsRequest& request, CAssetSettingsChangeResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ResetSettings( const CORE::CString& commandId, const CAssetSettingsRequest& request, CAssetSettingsChangeResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus BeginUploadFiles( const CORE::CString& commandId, const CORE::CString& name, const CAssetFileStatPage& files, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus AnswerPrompt( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult AnswerPromptAsync( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ImportWebSession( const CORE::CString& commandId, const CORE::CString& storeId, const AUTH::CSecretString& session, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ImportWebSessionAsync( const CORE::CString& commandId, const CORE::CString& storeId, const AUTH::CSecretString& session ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus WriteUploadBlock( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult WriteUploadBlockAsync( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus UnlockKeyring( const CORE::CString& commandId, const AUTH::CSecretString& code, CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult UnlockKeyringAsync( const CORE::CString& commandId, const AUTH::CSecretString& code ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteAssetStoreService > TEventCallback;
    typedef CORE::CTSharedPtr< CRemoteAssetStoreProvider, MT::CMutex > TRemoteProviderPtr;
    typedef GUCEF::map< CORE::CString, TAssetStoreProviderPtr > TProviderMap;

    /** One call on its way: the RPC calls it took (a read or a write of several blocks takes several) and its future */
    struct SPendingCall
    {
        enum EKind
        {
            KIND_CALL = 0 ,         /**< a generic call: one CallAnswer */
            KIND_WRITE              /**< WriteUploadBlock: CallAnswer per block */
        };

        EKind kind;
        CORE::CString operation;
        GUCEF::vector< CORE::CFutureResult > rpcFutures;
        CORE::CFutureResultSource source;
        bool decoded;                           /**< the answer is read, it waits for the events before it */
        TAssetStoreStatus status;
        CORE::CString message;
        CORE::CIDataNodeSerializable* result;   /**< owned until completion */
        UInt64 eventSeq;
        UInt64 decodedAtMs;

        SPendingCall( void );
    };
    typedef GUCEF::vector< SPendingCall* > TPendingVector;

    CRemoteAssetStoreService( const CRemoteAssetStoreService& src );                /**< not implemented */
    CRemoteAssetStoreService& operator=( const CRemoteAssetStoreService& src );     /**< not implemented */

    /** Waits for a future of this service and gives its record (also of a failure); the status of the call */
    template < class T >
    TAssetStoreStatus Await( const CORE::CFutureResult& future, T& result )
    {
        WaitForFuture( future );
        const TAssetStoreStatus status = GetStatusOfAnswer( future );
        CORE::CTaskPtr task = future.GetResult( 0 );
        const T* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< T >( task->GetTaskData() );
        if ( GUCEF_NULL != data )
            result = *data;
        else
        {
            result = T();
            CORE::CString message = GetMessageOfFuture( future );
            if ( message.IsNULLOrEmpty() && ASSETSTORE_STATUS_OK != status )
                message = "the call ended without an answer";
            SetFailureOf( &result, status, message );
        }
        return status;
    }

    void WaitForFuture( const CORE::CFutureResult& future );
    static TAssetStoreStatus GetStatusOfAnswer( const CORE::CFutureResult& future );
    static void SetFailureOf( CORE::CICloneable* result, TAssetStoreStatus status, const CORE::CString& message );

    /** A future that ended already */
    CORE::CFutureResult Refuse( const CORE::CString& operation, TAssetStoreStatus status, const CORE::CString& message );

    CORE::CFutureResult Track( SPendingCall* pending );
    bool IsPumpingThread( void ) const;
    UInt32 GetFilesChannel( void ) const;
    COMCORE::CRpcCallOptions MakeOptions( bool idempotent ) const;

    /** Completes the calls whose answers are in and whose events were raised; any thread */
    void ProcessPending( void );
    void Decode( SPendingCall& pending );
    void Complete( SPendingCall& pending );

    void UpdateStores( const TAssetStoreInfoVector& stores );
    void DetachProviders( void );

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnServiceStopping( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResync( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    static UInt64 NowMs( void );

    private:

    CAssetStoreServiceConfig m_config;
    CRemoteAssetStoreServiceSettings m_settings;
    bool m_settingsValid;
    AUTH::ICredentialStore* m_store;            /**< the store of the credentials: the configuration's or m_ownStore */
    AUTH::ICredentialStore* m_ownStore;         /**< a file store of credentialsDirectory, owned */
    DRNUTILS::CRemoteServiceConnection* m_connection;
    CORE::CTimer m_timer;
    TPendingVector m_pending;
    TProviderMap m_providers;
    TAssetStoreInfoVector m_stores;
    CORE::CString m_statusMessage;
    bool m_open;
    bool m_catalogueAvailable;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** The factory of the remote implementation, registered under "remote" by CAssetStoreGlobal */
typedef CORE::CTFactoryWithParam< CIAssetStoreService, CRemoteAssetStoreService, CAssetStoreServiceConfig, MT::CMutex > TRemoteAssetStoreServiceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H ? */
