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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCJOB_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "gucefCORE_CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */
#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#include "gucefASSETSTORE_CAssetJobEntry.h"
#define GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETJOBENTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#include "gucefASSETSTORE_CAssetLibrarySyncPlan.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H
#include "gucefASSETSTORE_CAssetLibrarySyncPlanner.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H ? */
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

class CLocalAssetStoreService;
class CPrimaryAssetStore;
class CAssetStoreDownloadManager;
class CAssetLibrarySyncWakeup;

/**
 *  One run of the library sync of a store (library sync design, section 5.2): a state machine on the pulse generator of
 *  the service, not a task that holds a thread of the pool. It lists every page of the library (QueryLibraryAsync(), one
 *  page in flight, the pacing is the store's), asks the details of the items the listing is not enough for (GetItemAsync(),
 *  one at a time), plans (CAssetLibrarySyncPlanner) and, for the mode "run", queues the downloads of the work list through
 *  the service (EnqueueDownload() with forIngest, the format, the revision the plan saw and the id of the job) in a window
 *  of items in flight that the admission rules bound (CAssetLibrarySyncAdmission). A slot frees when the ingest of the
 *  download ends (the events of the download manager and the store); nothing polls while it waits for them, a timer runs
 *  only while a call of the store is in flight or the admission waits. A failed download counts against the item
 *  (sync_items: attempts, the next attempt); the run goes on with the others. The run ends DONE when the work list is
 *  empty and nothing of it is in flight, then optionally starts the job index-pack over the packs without an index.
 *
 *  Start() and RequestCancel() may be called on any thread: the work is carried out on the thread of the pulse
 *  generator. FillEntry() and GetResult() are thread safe. The service raises its job events on SyncJobChangedEvent
 *  (event data: the id of the job as a CORE::TCloneableString).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncJob : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent SyncJobChangedEvent;

    static void RegisterEvents( void );

    /** The download in flight of an item of the work list */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SInFlight
    {
        CAssetLibrarySyncPlanner::SWorkItem work;
        bool downloaded;

        SInFlight( void );
    };
    typedef GUCEF::map< CORE::CString, SInFlight > TInFlightMap;

    /** What the run did: counts of the downloads of the work list */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SRunCounts
    {
        UInt32 queued;              /**< downloads queued */
        UInt32 downloaded;          /**< downloads that finished */
        UInt32 ingested;            /**< ingests that made a pack */
        UInt32 revalidated;         /**< downloads whose files were a pack already (the revision of that pack was confirmed) */
        UInt32 failed;              /**< downloads or ingests that failed */
        UInt32 cancelled;           /**< downloads cancelled by someone else */
        UInt32 skippedInFlight;     /**< work items left out at dispatch: a download of them was in flight already */
        UInt32 peakInFlight;        /**< the most items in flight at a time */
        UInt64 doneBytes;           /**< estimated bytes of the finished downloads */

        SRunCounts( void );
    };

    CAssetLibrarySyncJob( CLocalAssetStoreService& service, const CORE::CString& jobId, const CAssetLibrarySyncSpec& spec );

    virtual ~CAssetLibrarySyncJob() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetJobId( void ) const;
    const CAssetLibrarySyncSpec& GetSpec( void ) const;

    /** Checks the spec and the store and begins on the thread of the pulse generator; the reason when it cannot */
    TAssetStoreStatus Start( CORE::CString& problem );

    /** Stops queueing, cancels the downloads of the run that have not finished, ends CANCELLED; finished packs stay */
    void RequestCancel( void );

    /**
     *  Begins again a run that a restart of the service interrupted, from the checkpoint the run gave the service
     *  (CLocalAssetStoreService::NoteSyncCheckpoint()): with its work list cache it goes on queueing where it was and follows
     *  the downloads it had queued (those the download manager still holds); without the cache it plans again, and the
     *  downloads it had queued are in flight to the new plan, so nothing is queued twice. Called instead of Start()
     */
    TAssetStoreStatus Restore( const CORE::CDataNode& checkpoint, CORE::CString& problem );

    /** Stops queueing until RequestResume(); the downloads that are queued go on. Thread safe */
    void RequestPause( void );

    /** Takes up queueing after RequestPause(). Thread safe */
    void RequestResume( void );

    bool IsEnded( void ) const;
    TAssetJobState GetState( void ) const;
    TAssetStoreStatus GetStatus( void ) const;
    UInt32 GetFailedCount( void ) const;
    CORE::CString GetMessage( void ) const;
    CORE::CString GetResultText( void ) const;

    /** The progress of the run as members of the entry of the job list: state, state text, counts, bytes, the result in words */
    void FillEntry( CAssetJobEntry& entry ) const;

    /** The plan of the run (empty before the listing ended) and its counts */
    void GetPlan( CAssetLibrarySyncPlan& plan ) const;
    SRunCounts GetRunCounts( void ) const;

    /**
     *  The result of the job: the section "" or "plan" holds the counts of the plan and of the run, the storage and the
     *  failures; "items" holds the lines of the plan in pages (pageToken: the index of the first line)
     */
    TAssetStoreStatus GetResult( const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAssetLibrarySyncJob > TEventCallback;

    enum EPhase
    {
        PHASE_CREATED = 0   ,
        PHASE_LISTING       ,
        PHASE_ENRICHING     ,
        PHASE_RUNNING       ,
        PHASE_ENDED
    };

    enum EWait
    {
        WAIT_NONE = 0       ,
        WAIT_PAGE           ,
        WAIT_PAGE_RETRY     ,
        WAIT_ITEM           ,
        WAIT_ADMISSION      ,
        WAIT_SIGN_IN
    };

    struct SFailure
    {
        CORE::CString itemId;
        CORE::CString formatId;
        CORE::CString status;
        CORE::CString message;
    };

    void OnWake( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDownloadJobEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnIngestEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoreStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void Subscribe( void );
    void ConfigureAdmission( void );
    void ApplyPauseRequest( void );
    void ReconcileInFlight( void );
    void AdoptQueuedDownloads( void );
    bool HasCurrentPack( const CAssetLibrarySyncPlanner::SWorkItem& work ) const;
    void Checkpoint( bool force );
    bool WriteWorklist( void ) const;
    bool ReadWorklist( const CORE::CString& path );
    static void SerializeWorkItem( const CAssetLibrarySyncPlanner::SWorkItem& work, CORE::CDataNode& node );
    static void DeserializeWorkItem( const CORE::CDataNode& node, CAssetLibrarySyncPlanner::SWorkItem& work );

    void Begin( void );
    void RequestPage( void );
    void OnPageReady( void );
    void BeginPlanning( void );
    void RequestNextEnrichment( void );
    void BuildPlan( void );
    void Fill( void );
    void Finish( void );
    void Cancel( void );
    void End( TAssetJobState state, TAssetStoreStatus status, const CORE::CString& message );
    void SetWait( EWait wait, UInt32 intervalMs );
    void SetStateText( const CORE::CString& text );
    void Changed( void );
    void ItemEnded( const CORE::CString& downloadJobId, TAssetStoreStatus status, const CORE::CString& message, bool succeeded );
    void RecordAttempt( const CAssetLibrarySyncPlanner::SWorkItem& work, TAssetStoreStatus status, const CORE::CString& message, bool succeeded );
    CORE::CString ResolveVolumePath( void ) const;
    CAssetLibrarySyncAdmission::SLoad GetLoad( void ) const;
    CORE::CString DescribeRun( void ) const;
    CORE::CString GetResultTextLocked( void ) const;

    CAssetLibrarySyncJob( const CAssetLibrarySyncJob& src );              /**< not implemented */
    CAssetLibrarySyncJob& operator=( const CAssetLibrarySyncJob& src );   /**< not implemented */

    private:

    CLocalAssetStoreService& m_service;
    CORE::CString m_jobId;
    CAssetLibrarySyncSpec m_spec;
    TAssetStoreProviderPtr m_provider;
    CPrimaryAssetStore* m_store;
    CAssetStoreDownloadManager* m_manager;
    CAssetLibrarySyncWakeup* m_wakeup;
    CORE::CTimer* m_timer;
    MT::CTAtomic< UInt32 > m_cancelRequested;
    MT::CTAtomic< UInt32 > m_pauseRequest;          /**< 0 none, 1 pause, 2 resume: taken up on the thread of the pulse generator */
    bool m_paused;
    bool m_subscribed;
    bool m_restoredRun;                             /**< Restore() took the work list from the cache: Begin() goes on queueing */
    CORE::CString m_lastCheckpoint;                 /**< what the last checkpoint said, so an unchanged one is not written again */
    CORE::CString m_lastQueuedTitle;                /**< the title of the item queued last, for the progress text */

    // the thread of the pulse generator only
    EPhase m_phase;
    EWait m_wait;
    CORE::CFutureResult m_future;
    CORE::CString m_cursor;
    UInt32 m_pageRetries;
    Int64 m_retryAtMs;
    CORE::CString::StringVector m_enrichment;
    size_t m_nextEnrichment;
    CAssetLibrarySyncPlanner m_planner;
    CAssetLibrarySyncAdmission m_admission;
    CAssetLibrarySyncPlanner::TWorkItemVector m_worklist;
    size_t m_next;
    TInFlightMap m_inFlight;

    // shared with the readers, under the lock
    mutable MT::CMutex m_dataLock;
    TAssetJobState m_state;
    TAssetStoreStatus m_status;
    CORE::CString m_stateText;
    CORE::CString m_message;
    CORE::CString m_indexJobId;
    CAssetLibrarySyncPlan m_plan;
    SRunCounts m_counts;
    GUCEF::vector< SFailure > m_failures;
    UInt64 m_storageFreeBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCJOB_H ? */
