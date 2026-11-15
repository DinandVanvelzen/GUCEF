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

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */
#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHENGINE_H
#include "gucefASSETSTORE_CAssetSearchEngine.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHENGINE_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHENGINE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H
#include "gucefASSETSTORE_CAssetSavedSearch.h"
#define GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H
#endif /* GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#include "gucefASSETSTORE_CAssetIngestResult.h"
#define GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTRESULT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTHISTORYENTRY_H
#include "gucefASSETSTORE_CAssetIngestHistoryEntry.h"
#define GUCEF_ASSETSTORE_CASSETINGESTHISTORYENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTHISTORYENTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#include "gucefASSETSTORE_CAssetConversionOutcome.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVERIFICATIONREPORT_H
#include "gucefASSETSTORE_CAssetVerificationReport.h"
#define GUCEF_ASSETSTORE_CASSETVERIFICATIONREPORT_H
#endif /* GUCEF_ASSETSTORE_CASSETVERIFICATIONREPORT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#include "gucefASSETSTORE_CAssetStoreDownloadJob.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
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

class CAssetIngestPipeline;
class CAssetPackConversion;
class CIAssetConverterHook;
class CAssetStoreDownloadManager;
class CPrimaryAssetStoreOperationTaskConsumer;

/**
 *  The primary store (sections 5 to 9 of the design): the main copy of the assets of the owner, fed by the online
 *  stores and by packs on disk, in one normalised form, with its catalogue and index in a database.
 *
 *  Open() opens the catalogue on the database of the settings (any COMCORE::CIDatabaseClient backend: "postgresql",
 *  "vfstables"), runs its migrations with the seed data of the module, loads the vocabulary, mounts the locations that
 *  name a folder, checks which locations are reachable and repairs packs whose indexing was interrupted (without
 *  transactions a pack is written pack row first with the state "indexing", then its assets and what refers to them,
 *  then the pack row again: a pack still "indexing" at the next open has its rows taken out and is left failed, to be
 *  resumed). The password of the database comes from the credential store given with SetCredentialStore(), or the
 *  password file of the settings.
 *
 *  Ingest (section 8) runs the steps stage, identify, normalise, analyse, convert, place, index and record on the
 *  calling thread (Ingest()) or as a pool task (IngestAsync()); a failed step leaves the pack staged with its reason,
 *  ResumeIngest() continues from that step. Packs of downloads made with EnqueueDownload() land in the staging area of
 *  the location; with an attached download manager every completed download of a source with the automatic policy is
 *  ingested by itself.
 *
 *  Conversion (section 7.4): with a converter hook (SetConverterHook()) the convert step of an ingest converts a pack
 *  whose keep policy asks for converted content. A pack the converter refuses (its licence record does not allow it)
 *  is ingested as original and gets the state "conversion refused", one whose conversion ended with errors the state
 *  "conversion pending"; neither is a failed ingest, the reason is in the state reason and in the ingest history. A
 *  pack that is "conversion pending" (also: no converter at the time of the ingest) can be converted later with
 *  ConvertPack() or ConvertPendingPacks(), as a job on the calling thread or as a pool task (ConvertPackAsync(): the
 *  future's RequestCancellation() stops it, GetResult() has a CAssetConversionOutcome). The conversion of a pack that
 *  is placed is written to "converted/<profile>/" of the pack, the converted paths and formats of its assets and the
 *  versions of the converter parts that made it (pack.conversion) go into the catalogue, and IsConversionStale() says
 *  when a part of the converter has another version since. Progress and the end of a conversion are the events
 *  ConversionProgressEvent, ConversionFinishedEvent and ConversionFailedEvent (CAssetConversionEventData); those of the
 *  convert step of an ingest have the id of the ingest as their job.
 *
 *  Search (section 9.4) through CAssetSearchEngine; the saved searches with the precanned set of section 9.6.
 *  Locations (section 9.7): a location that is not reachable marks its packs unavailable, a location can be relocated
 *  to another root and verified (every file and its digest) as a background task.
 *
 *  Licence restriction flags: GetPacksAllowingUse() and IsUseAllowed() let learning based features leave out packs
 *  with the flag NoMachineLearning; ExcludeRestricted() adds the exclusion to a search query.
 *
 *  It is a store provider as well ("primary"): the library is the packs, an item is a pack with its files as one
 *  format "original" (and "converted"). Events, raised on the thread of the pulse generator of the store:
 *  IngestProgressEvent (a step completed), IngestFinishedEvent (ingested or duplicate), IngestFailedEvent (event data
 *  CAssetIngestEventData) and LocationStateChangedEvent (TCloneableString: the location id). Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CPrimaryAssetStore : public CIAssetStoreProvider ,
                                                       public CORE::CIConfigurable
{
    public:

    static const CORE::CString ProviderId;                  /**< "primary" */

    static const CORE::CEvent IngestProgressEvent;
    static const CORE::CEvent IngestFinishedEvent;
    static const CORE::CEvent IngestFailedEvent;
    static const CORE::CEvent ConversionProgressEvent;      /**< CAssetConversionEventData: a part of the assets is converted */
    static const CORE::CEvent ConversionFinishedEvent;      /**< CAssetConversionEventData: converted, or refused (the status says) */
    static const CORE::CEvent ConversionFailedEvent;        /**< CAssetConversionEventData: failed or cancelled (the status says) */
    static const CORE::CEvent LocationStateChangedEvent;

    static void RegisterEvents( void );

    CPrimaryAssetStore( void );
    virtual ~CPrimaryAssetStore() GUCEF_VIRTUAL_OVERRIDE;

    void SetSettings( const CPrimaryAssetStoreSettings& settings );
    CPrimaryAssetStoreSettings GetSettings( void ) const;

    /** Where the password of the catalogue database is kept, not owned; GUCEF_NULL for none */
    void SetCredentialStore( AUTH::ICredentialStore* credentialStore );

    /** The conversion step of the ingest, not owned; GUCEF_NULL (the default): packs that want conversion are marked pending */
    void SetConverterHook( CIAssetConverterHook* converterHook );

    /** Opens the catalogue up to the given schema version (the current one by default) */
    TAssetStoreStatus Open( UInt32 targetSchemaVersion = CAssetCatalogue::CurrentSchemaVersion );

    void Close( void );

    bool IsOpen( void ) const;

    /** The message of the last failure */
    CORE::CString GetLastErrorMessage( void ) const;

    CAssetCatalogue& GetCatalogue( void );
    CAssetVocabulary& GetVocabulary( void );

    // storage locations

    /** Checks which locations are reachable, raising LocationStateChangedEvent for those that changed */
    void RefreshLocationStates( void );

    bool IsLocationOnline( const CORE::CString& locationId ) const;

    /** The VFS root of the location, empty when there is no such location */
    CORE::CString GetLocationRoot( const CORE::CString& locationId ) const;

    /** Moves a location to another VFS root (and mount folder, when given): one row and the settings change */
    TAssetStoreStatus RelocateLocation( const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath = CORE::CString::Empty );

    /** Checks every file of every pack of the location: existence and digest */
    TAssetStoreStatus VerifyLocation( const CORE::CString& locationId, CAssetVerificationReport& report );
    CORE::CFutureResult VerifyLocationAsync( const CORE::CString& locationId );

    /** A new VFS folder in the staging area of the location (the default location when empty) for a download */
    CORE::CString MakeDownloadStagingPath( const CORE::CString& locationId = CORE::CString::Empty ) const;

    // ingest

    TAssetStoreStatus Ingest( const CAssetIngestRequest& request, CAssetIngestResult& result );
    CORE::CFutureResult IngestAsync( const CAssetIngestRequest& request );

    TAssetStoreStatus ResumeIngest( const CORE::CString& ingestId, CAssetIngestResult& result );
    CORE::CFutureResult ResumeIngestAsync( const CORE::CString& ingestId );

    /** The ingests that are staged and did not finish (their manifests in the staging areas) */
    void GetPendingIngests( CORE::CString::StringVector& ingestIds ) const;

    // conversion of packs that are in the store

    /** Converts a pack that is placed in the store ("conversion pending", or again); see the class description. The status is the one of the outcome */
    TAssetStoreStatus ConvertPack( const CORE::CString& packId, CAssetConversionOutcome& outcome );
    CORE::CFutureResult ConvertPackAsync( const CORE::CString& packId );

    /** Converts every pack that is "conversion pending", one after the other; the outcome has the counts of the packs */
    TAssetStoreStatus ConvertPendingPacks( CAssetConversionOutcome& outcome );
    CORE::CFutureResult ConvertPendingPacksAsync( void );

    /**
     *  Raises ConversionProgressEvent, ConversionFinishedEvent or ConversionFailedEvent (CAssetConversionEventData) on the thread
     *  of the pulse generator of the store: what the conversion of a pack reports as it goes; for the code that carries out the
     *  conversions (the ingest pipeline and CAssetPackConversion), and for converter hooks and tests that stand in for them
     */
    void NotifyConversionEvent( const CORE::CEvent& eventId, const CORE::CString& jobId, const CORE::CString& packId, bool forIngest,
                                UInt32 done, UInt32 total, const CORE::CString& assetPath, TAssetStoreStatus status, const CORE::CString& message );

    /** The ids of the packs whose state is "conversion pending" */
    void GetPacksPendingConversion( CORE::CString::StringVector& packIds ) const;

    /** Whether the conversion of the pack was made by other versions of the converter parts than the ones now; false for a pack that was not converted */
    TAssetStoreStatus IsConversionStale( const CORE::CString& packId, bool& stale, CORE::CString& reason ) const;

    /** The ids of the packs whose conversion is stale */
    TAssetStoreStatus GetStaleConversions( CORE::CString::StringVector& packIds ) const;

    /** Takes a pack out of the catalogue; with 'deleteFiles' its placed files go as well */
    TAssetStoreStatus RemovePack( const CORE::CString& packId, bool deleteFiles );

    // downloads for the primary store

    /** Ingests completed downloads of sources with the automatic policy; GUCEF_NULL detaches. Not owned */
    void AttachDownloadManager( CAssetStoreDownloadManager* downloadManager );

    /** Queues a download of the store item into the staging area, marked for the primary store */
    TAssetStoreStatus EnqueueDownload( CAssetStoreDownloadManager& downloadManager, const CORE::CString& providerId, const CORE::CString& itemId,
                                       const CORE::CString& formatId, const CORE::CString::StringVector& fileIds, CORE::CString& jobId );

    /** The ingest request for a completed download job */
    static CAssetIngestRequest MakeIngestRequest( const CAssetStoreDownloadJob& job );

    // search

    TAssetStoreStatus Search( const CAssetSearchQuery& query, CAssetSearchResultPage& page ) const;
    CORE::CFutureResult SearchAsync( const CAssetSearchQuery& query );

    TAssetStoreStatus Suggest( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& keywords ) const;

    TAssetStoreStatus GetSavedSearches( TAssetSavedSearchVector& searches ) const;
    TAssetStoreStatus SaveSearch( const CAssetSavedSearch& search );
    TAssetStoreStatus DeleteSavedSearch( const CORE::CString& searchId );
    TAssetStoreStatus RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& page ) const;

    // records

    TAssetStoreStatus GetPack( const CORE::CString& packId, CAssetPackRecord& pack ) const;
    TAssetStoreStatus GetAssets( const CORE::CString& packId, TAssetRecordVector& assets ) const;
    TAssetStoreStatus GetIngestHistory( GUCEF::vector< CAssetIngestHistoryEntry >& entries, const CORE::CString& packId = CORE::CString::Empty ) const;

    /** The VFS path of the file of an asset; UNAVAILABLE with the location in the message when it is not reachable */
    TAssetStoreStatus GetAssetVfsPath( const CORE::CString& assetId, CORE::CString& vfsPath, CORE::CString& message ) const;

    /** The VFS path of the converted file of an asset; NOT_FOUND when the asset was not converted, UNAVAILABLE as GetAssetVfsPath() */
    TAssetStoreStatus GetAssetConvertedVfsPath( const CORE::CString& assetId, CORE::CString& vfsPath, CORE::CString& message ) const;

    // licence restrictions

    /** The packs whose licence does not have the restriction flag, for features that must respect it */
    TAssetStoreStatus GetPacksAllowingUse( const CORE::CString& restrictionFlag, CORE::CString::StringSet& packIds ) const;

    /** Whether the licence of the pack is without the restriction flag */
    TAssetStoreStatus IsUseAllowed( const CORE::CString& packId, const CORE::CString& restrictionFlag, bool& allowed ) const;

    /** Adds the exclusion of packs with the flag to a search query */
    static void ExcludeRestricted( CAssetSearchQuery& query, const CORE::CString& restrictionFlag );

    // CIAssetStoreProvider

    virtual const CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetIconName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus GetItem( const CORE::CString& itemId, CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId, const CORE::CString::StringVector& fileIds, CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE;

    // CIConfigurable: the settings as a "primaryStore" object

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CAssetIngestPipeline;
    friend class CAssetPackConversion;
    friend class CPrimaryAssetStoreOperationTaskConsumer;

    typedef CORE::CTEventHandlerFunctor< CPrimaryAssetStore > TEventCallback;

    TAssetStoreStatus Fail( TAssetStoreStatus status, const CORE::CString& message ) const;
    TAssetStoreStatus RunIngest( const CAssetIngestRequest& request, CAssetIngestResult& result, const CORE::CFutureResultSource* cancel );
    TAssetStoreStatus RunResumeIngest( const CORE::CString& ingestId, CAssetIngestResult& result, const CORE::CFutureResultSource* cancel );
    TAssetStoreStatus RunConvertPack( const CORE::CString& packId, CAssetConversionOutcome& outcome, const CORE::CFutureResultSource* cancel );
    TAssetStoreStatus RunConvertPending( CAssetConversionOutcome& outcome, const CORE::CFutureResultSource* cancel );
    TAssetStoreStatus RepairInterruptedPacks( void );
    TAssetStoreStatus RemovePackRows( const CORE::CString& packId );
    void SyncLocationAndSourceRows( void );
    CORE::CFutureResult QueueOperation( CORE::CICloneable* operationData );
    void OperationEnded( void );
    void NotifyIngestEvent( const CORE::CEvent& eventId, const CORE::CString& ingestId, const CORE::CString& packId,
                            TAssetIngestStep step, TAssetStoreStatus status, const CORE::CString& message );
    CAssetSearchEngine::TLocationStateMap GetLocationStates( void ) const;
    CIAssetConverterHook* GetConverterHook( void ) const;
    MT::CMutex& GetIndexLock( void );
    CORE::CString FindLocationOfVfsPath( const CORE::CString& vfsPath ) const;
    static CAssetPackRecord PackFromRow( const CAssetCatalogue::TRow& row );
    static CAssetRecord AssetFromRow( const CAssetCatalogue::TRow& row );

    void OnDownloadJobFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CPrimaryAssetStore( const CPrimaryAssetStore& src );              /**< not implemented */
    CPrimaryAssetStore& operator=( const CPrimaryAssetStore& src );   /**< not implemented */

    private:

    CPrimaryAssetStoreSettings m_settings;
    AUTH::ICredentialStore* m_credentialStore;
    CIAssetConverterHook* m_converterHook;
    CAssetStoreDownloadManager* m_downloadManager;
    CAssetCatalogue m_catalogue;
    CAssetVocabulary m_vocabulary;
    CAssetSearchEngine m_searchEngine;
    GUCEF::map< CORE::CString, bool > m_locationOnline;
    CORE::CString::StringSet m_mountedLocations;
    MT::CTAtomic< UInt32 > m_pendingOperations;
    mutable CORE::CString m_lastError;
    mutable MT::CMutex m_lock;
    MT::CMutex m_indexLock;
};

typedef CORE::CTSharedPtr< CPrimaryAssetStore, MT::CMutex > TPrimaryAssetStorePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
