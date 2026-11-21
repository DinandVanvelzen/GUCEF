/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CLIBRARYSYNCCONTROLLER_H
#define ASSETWORKBENCHLIB_CLIBRARYSYNCCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Drives the library sync of the online stores through the asset store service (library sync design, section 7.4): per store the
 *  plan (a dry run, the pipeline job "sync-library" in the mode "plan", whose result gives the counts: missing, stale, current,
 *  unavailable, the estimate of the download and the free disk), the sync (the same job in the mode "run", started without a
 *  confirmation as the owner decided), its progress (the job events of the service: downloads done of the downloads planned, the
 *  state text with the item queued last), cancel, the retry of the failed items and the summary of the last run. A download of
 *  several items of a store (the selection of the Sources pane) is a sync of those items alone (the spec's itemIds), so it gets the
 *  same plan, formats and admission. The runs take the profile the service keeps for the store (useStoreProfile).
 *
 *  With a service in this process the calls are made at once. With a service that is not in process they are asked, the answers are
 *  taken by Update() on the thread of the interface, and nothing waits. A sync that runs already on the service (started by another
 *  client, or before a reconnect) is found by RefreshStores() and followed.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CLibrarySyncController : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CLibrarySyncController > TEventCallback;

    static const CORE::CEvent StoresChangedEvent;       /**< no data: the stores were read again */
    static const CORE::CEvent StoreChangedEvent;        /**< TCloneableString: the id of a store whose plan, progress or result changed */
    static const CORE::CEvent PlanReadyEvent;           /**< TCloneableString: the id of a store whose plan is there */
    static const CORE::CEvent JobStartedEvent;          /**< TCloneableString: the id of a job the controller started or found */
    static const CORE::CEvent NotificationEvent;        /**< TCloneableString: the text of the end of a sync it follows */

    static void RegisterEvents( void );

    /** What the controller knows of the sync of one store */
    struct ASSETWORKBENCHLIB_PUBLIC_CPP SStoreState
    {
        CORE::CString storeId;
        CORE::CString displayName;
        bool signedIn;

        CORE::CString planJobId;            /**< the dry run that was asked last */
        bool planning;
        bool planned;                       /**< the counts below are those of a plan */
        UInt32 listed;
        UInt32 missing;
        UInt32 stale;
        UInt32 current;
        UInt32 unavailable;
        UInt32 queuedDownloads;
        UInt64 estimatedBytes;
        UInt64 freeBytes;                   /**< of the volume of the location, 0 when not known */
        CORE::CString storageVerdict;       /**< "fits", "does-not-fit" or empty */

        CORE::CString runJobId;             /**< the run that was started or found last */
        bool running;
        ASSETSTORE::TAssetJobState runState;
        UInt32 itemsDone;
        UInt32 itemsTotal;
        UInt32 failed;                      /**< failed downloads of the last run that ended (its result) */
        bool retryAvailable;                /**< the service would retry the failed items of the last run */
        Float32 fraction;
        CORE::CString progressText;         /**< the state text of the job: the counts and the item queued last */
        CORE::CString lastResult;           /**< the summary of the last run that ended */
        CORE::CString problem;              /**< why the last request of this store was refused, empty when it was not */

        SStoreState( void );
    };
    typedef GUCEF::vector< SStoreState > TStoreStateVector;

    explicit CLibrarySyncController( ASSETSTORE::CIAssetStoreService& service );

    virtual ~CLibrarySyncController() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the online stores of the service and the sync jobs that run on it; false when it cannot be asked */
    bool RefreshStores( void );

    UInt32 GetStoreCount( void ) const;
    const SStoreState* GetStore( UInt32 index ) const;
    const SStoreState* FindStore( const CORE::CString& storeId ) const;

    /** Starts the dry run of the store: its plan is there with PlanReadyEvent; false with GetProblem() when it is refused */
    bool RequestPlan( const CORE::CString& storeId );

    /** Starts the sync of the whole library of the store, at once */
    bool StartSync( const CORE::CString& storeId );

    /** Starts a sync of the items alone (a download of a selection) */
    bool DownloadItems( const CORE::CString& storeId, const CORE::CString::StringVector& itemIds );

    /** Cancels the run of the store (or its dry run when no run is followed) */
    bool Cancel( const CORE::CString& storeId );

    /** Starts the failed items of the last run of the store again, their counters reset */
    bool RetryFailed( const CORE::CString& storeId );

    /** Takes the answers of a service that is not in process; call it between two frames */
    void Update( void );

    /** Whether answers of the service are awaited */
    bool IsWaiting( void ) const;

    const CORE::CString& GetProblem( void ) const;

    /** The ends of runs that were notified, oldest first */
    const CORE::CString::StringVector& GetNotifications( void ) const;

    /** A size for people: "1.5 GiB" */
    static CORE::CString FormatBytes( UInt64 bytes );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EPendingKind
    {
        PENDING_STORES = 0  ,
        PENDING_JOBS        ,
        PENDING_START_PLAN  ,
        PENDING_START_RUN   ,
        PENDING_RESULT_PLAN ,
        PENDING_RESULT_RUN  ,
        PENDING_CONTROL
    };

    struct SPending
    {
        CORE::CFutureResult future;
        CORE::CString storeId;
        EPendingKind kind;

        SPending( void );
    };
    typedef GUCEF::vector< SPending > TPendingVector;

    CLibrarySyncController( const CLibrarySyncController& src );              /**< not implemented */
    CLibrarySyncController& operator=( const CLibrarySyncController& src );   /**< not implemented */

    void OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoreStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    SStoreState* FindStoreState( const CORE::CString& storeId );
    SStoreState& GetOrAddStoreState( const CORE::CString& storeId );
    bool StartJob( const CORE::CString& storeId, const ASSETSTORE::CAssetLibrarySyncSpec& spec );
    void TakeStarted( const CORE::CString& storeId, bool plan, ASSETSTORE::TAssetStoreStatus status, const ASSETSTORE::CAssetServiceResult& result );
    void TakeStores( const ASSETSTORE::CAssetStoreInfoPage& page );
    void TakeJobs( const ASSETSTORE::CAssetJobPage& page );
    void TakeJob( const ASSETSTORE::CAssetJobEntry& job );
    void AskResult( const CORE::CString& storeId, const CORE::CString& jobId, bool plan );
    void TakeResult( const CORE::CString& storeId, bool plan, const ASSETSTORE::CAssetServiceDocument& result );
    bool Control( const CORE::CString& jobId, ASSETSTORE::TAssetJobAction action );
    CORE::CString NextCommandId( void );
    void StoreChanged( const CORE::CString& storeId );

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    TStoreStateVector m_stores;
    TPendingVector m_pending;
    CORE::CString::StringVector m_notifications;
    CORE::CString m_problem;
    UInt32 m_commandCounter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CLIBRARYSYNCCONTROLLER_H ? */
