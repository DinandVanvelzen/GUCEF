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

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H
#define ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H

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

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#include "AssetWorkbenchLib_CBulkConversionSettings.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H
#include "AssetWorkbenchLib_CBulkConversionPlan.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H ? */

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
 *  Drives a bulk conversion through the asset store service: the plan (the dry run, operation convert_plan, per pack of the scope), the
 *  start (StartPipelineJob, a job of the service: in a remote service it runs on the server and survives a disconnect of the client,
 *  the job is found again by its id), the follow-up of the jobs it started (progress is the job's own; a completion raises
 *  NotificationEvent and is logged), and resume, retry of the failed assets and cancel of a job (ControlJob).
 *
 *  With a service in this process the calls are made at once. With a service that is not in process they are asked, the answers are
 *  taken by Update() on the thread of the interface, and nothing waits. A scope that is a saved search is resolved through
 *  RunSavedSearch(): its result is the packs of the hits (all of each pack).
 *
 *  A job is started from the plan: Start() works on the packs the last plan says have work and are allowed, and skips (and says so)
 *  the packs that the licence refuses. A pack that has no work is not started.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CBulkConversionController : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CBulkConversionController > TEventCallback;

    static const CORE::CEvent PlanReadyEvent;           /**< no data: the plan is there (valid or with the reason it is not) */
    static const CORE::CEvent JobStartedEvent;          /**< TCloneableString: the id of a job the controller started */
    static const CORE::CEvent NotificationEvent;        /**< TCloneableString: the text of a completion of a job it follows */
    static void RegisterEvents( void );

    struct SPackScope
    {
        CORE::CString packId;
        CORE::CString::StringVector assets;     /**< empty: all of the pack */
        bool refused;                           /**< the licence refuses the conversion */
        UInt32 workCount;
        SPackScope( void );
    };
    typedef GUCEF::vector< SPackScope > TPackScopeVector;

    explicit CBulkConversionController( ASSETSTORE::CIAssetStoreService& service );

    virtual ~CBulkConversionController() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Asks for the plan of the settings: the scope is resolved and every pack of it planned. With a service in this process the plan is
     *  there when the call returns; false (with GetProblem()) when the settings are not valid. IsPlanning() tells the asked state
     */
    bool RequestPlan( const CBulkConversionSettings& settings );

    bool IsPlanning( void ) const;

    /** The plan of all the packs of the scope together: the counts added, the items of every pack in the order of the packs */
    const CBulkConversionPlan& GetPlan( void ) const;

    const TPackScopeVector& GetPackScopes( void ) const;

    /** The settings the plan was made for */
    const CBulkConversionSettings& GetPlannedSettings( void ) const;

    /** Whether Start() would start a job: the plan is there, has work and is not refused for every pack */
    bool CanStart( void ) const;

    /**
     *  Starts a job for every pack of the plan that has work (the job of a pack of the plan that the licence refuses is not started).
     *  The number of jobs started or asked for; 0 with GetProblem() when there is nothing to start
     */
    UInt32 Start( void );

    /** The jobs that were started, by id (the answers of a remote service come with Update()) */
    const CORE::CString::StringVector& GetJobIds( void ) const;

    /** Follows a job of the service that this controller did not start (a job of the server found again by its id) */
    void TrackJob( const CORE::CString& jobId );

    /** The job of a pack that was cancelled or failed goes on where it stopped (the cache keeps what was done) */
    bool ResumeJob( const CORE::CString& jobId );

    /** The failed assets of a job alone, again */
    bool RetryFailedOfJob( const CORE::CString& jobId );

    bool CancelJob( const CORE::CString& jobId );

    /** Takes the answers of a service that is not in process; call it between two frames */
    void Update( void );

    const CORE::CString& GetProblem( void ) const;

    /** The completions that were notified, oldest first */
    const CORE::CString::StringVector& GetNotifications( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EStage
    {
        STAGE_IDLE = 0      ,
        STAGE_RESOLVING     ,   /**< the saved search is being read */
        STAGE_PLANNING
    };

    CBulkConversionController( const CBulkConversionController& src );
    CBulkConversionController& operator=( const CBulkConversionController& src );

    void OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ResolveScope( void );
    void BeginPlanning( void );
    void TakePlan( size_t index, const ASSETSTORE::CAssetServiceDocument& answer );
    void FinishPlanning( void );
    bool IsTracked( const CORE::CString& jobId ) const;
    UInt32 StartOne( const SPackScope& scope );
    void SendControl( const CORE::CString& jobId, ASSETSTORE::TAssetJobAction action, const CORE::CString& what );

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    CBulkConversionSettings m_settings;
    TPackScopeVector m_scopes;
    CBulkConversionPlan m_plan;
    GUCEF::vector< CBulkConversionPlan > m_packPlans;
    GUCEF::vector< CORE::CFutureResult > m_planFutures;
    CORE::CFutureResult m_resolveFuture;
    EStage m_stage;
    CORE::CString::StringVector m_jobIds;
    CORE::CString::StringVector m_notified;
    CORE::CString::StringVector m_notifications;
    GUCEF::vector< CORE::CFutureResult > m_startFutures;
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

#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H ? */
