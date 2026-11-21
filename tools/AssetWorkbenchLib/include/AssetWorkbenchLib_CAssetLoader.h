/*
 *  AssetWorkbenchLib: opening an asset without blocking the interface
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

#ifndef ASSETWORKBENCHLIB_CASSETLOADER_H
#define ASSETWORKBENCHLIB_CASSETLOADER_H

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

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#include "AssetWorkbenchLib_CWorkbenchJobRegistry.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H ? */

#ifndef ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#include "AssetWorkbenchLib_CIAssetLoadJob.h"
#define ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#endif /* ASSETWORKBENCHLIB_CIASSETLOADJOB_H ? */

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
 *  Opens one asset at a time without blocking the thread of the interface (feedback item F3). Begin() queues the job's
 *  Prepare() on a thread of a pool and answers the CFutureResult of that task; Update(), called every frame on the thread of the
 *  renderer, sees when it is ready and then runs the job's Step() pieces within a time budget per frame, so Ogre and GPU objects
 *  are only made there, a piece at a time. When it is complete LoadFinishedEvent follows.
 *
 *  Stale results: only the newest request counts. A Begin() while another request is in flight ends that one (cancel requested of
 *  its task; when its Prepare() returns the job is discarded on the thread of Update(), and counted by GetDroppedCount()); so a
 *  result that arrives after the user opened something else never reaches the views. The same goes for Cancel() and for a cancel
 *  of the user through the activity service.
 *
 *  Feedback: each request is an operation of the CActivityService (label given to Begin(), cancellable), so it follows the
 *  rule of feedback within 300 ms. Without a service there is no feedback and no cancel button.
 *
 *  Run() is the synchronous form for tests and the command line: the same two halves on the calling thread, no pool.
 *  The threading rule is kept: nothing here waits for another thread or runs the loop of another, except the destructor, which
 *  waits for the tasks it started (they stop at their next look at the control).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetLoader : public CORE::CObservingNotifier
{
    public:

    /** A request is over; TCloneableUInt32: the id of the request. GetLastOutcome() says how */
    static const CORE::CEvent LoadFinishedEvent;

    static void RegisterEvents( void );

    /** The pool that the preparations run in unless SetThreadPoolName() says another: one thread, so one preparation at a time */
    static const CORE::CString DefaultThreadPoolName;

    enum EOutcome
    {
        OUTCOME_NONE = 0    ,
        OUTCOME_SUCCEEDED   ,
        OUTCOME_FAILED      ,
        OUTCOME_CANCELLED       /**< by the user or by Cancel() */
    };

    enum EDefaults
    {
        DefaultSliceBudgetInMs = 8      /**< the time per frame that Update() spends on the pieces of the renderer, at least one piece is made */
    };

    /** The activity service is not owned and may be NULL */
    explicit CAssetLoader( GUI::CActivityService* activity );

    virtual ~CAssetLoader() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The registry the requests are jobs of (not owned, NULL for none): a request that runs longer than the registry says (300 ms) is a job of
     *  the jobs list with the kind of DescribeNext() (or the kind given here), cancellable through the list
     */
    void SetJobRegistry( CWorkbenchJobRegistry* registry, const CORE::CString& defaultKind );

    /** What the next Begin() is as a job: its kind, what it does within the kind, and what it works on; the default kind applies again after it */
    void DescribeNext( const CORE::CString& kind, const CORE::CString& subKind, const CORE::CString& scope );

    /** The name of the thread pool the preparation runs in; DefaultThreadPoolName when empty */
    void SetThreadPoolName( const CORE::CString& name );

    /**
     *  Starts a request; one that is in flight is ended (see above). The label is what the user sees while it takes time.
     *  The id of the request is GetCurrentRequest() afterwards. The future is that of the preparation task, with no future when it
     *  could not be queued (the request then fails at the next Update())
     */
    CORE::CFutureResult Begin( const TAssetLoadJobPtr& job, const CORE::CString& label, UInt32 expectedDurationInMs = 0 );

    /** The request in flight, 0 for none */
    UInt32 GetCurrentRequest( void ) const;

    bool IsBusy( void ) const;

    /** The activity operation of the request in flight, 0 for none */
    UInt32 GetCurrentActivity( void ) const;

    /** Ends the request in flight: its result is dropped; LoadFinishedEvent follows with OUTCOME_CANCELLED */
    void Cancel( void );

    /** On the thread of the renderer: reaps what ended, and makes the pieces of the job in flight within the budget (in milliseconds) */
    void Update( UInt32 sliceBudgetInMs = (UInt32) DefaultSliceBudgetInMs );

    /** The synchronous form: Prepare() and all Step()s on the calling thread. False with GetLastProblem() when it failed */
    bool Run( const TAssetLoadJobPtr& job, const CORE::CString& label );

    /** The request that ended last, how, and why when it failed */
    UInt32 GetLastFinishedRequest( void ) const;

    EOutcome GetLastOutcome( void ) const;

    const CORE::CString& GetLastProblem( void ) const;

    /** The number of results that were dropped because they arrived for a request that was ended meanwhile */
    UInt32 GetDroppedCount( void ) const;

    /** The number of Step() calls that were made for the last finished request, to see that the work is done in pieces */
    UInt32 GetLastStepCount( void ) const;

    protected:

    /** The destruction of the activity service: the request goes on without feedback (the standard events reach only this) */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAssetLoader > TEventCallback;

    enum ERequestState
    {
        STATE_PREPARING = 0 ,
        STATE_STEPPING
    };

    struct SRequest
    {
        UInt32 id;
        TAssetLoadJobPtr job;
        CORE::CFutureResult future;
        UInt32 activityId;
        UInt32 jobNumber;
        ERequestState state;
        UInt32 stepCount;
        SRequest( void );
    };
    typedef GUCEF::vector< SRequest > TRequestVector;

    void Finish( EOutcome outcome, const CORE::CString& problem );

    /** Takes the request in flight out of the way: a job that still prepares waits in the list of those to discard */
    void Abandon( void );

    void OnCancelRequested( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    CAssetLoader( const CAssetLoader& src );
    CAssetLoader& operator=( const CAssetLoader& src );

    private:

    GUI::CActivityService* m_activity;
    CWorkbenchJobRegistry* m_registry;
    CORE::CString m_defaultKind;
    CORE::CString m_nextKind;
    CORE::CString m_nextSubKind;
    CORE::CString m_nextScope;
    CORE::CString m_threadPoolName;
    SRequest m_current;
    bool m_hasCurrent;
    TRequestVector m_ended;
    UInt32 m_nextRequestId;
    UInt32 m_lastFinished;
    EOutcome m_lastOutcome;
    CORE::CString m_lastProblem;
    UInt32 m_lastStepCount;
    UInt32 m_droppedCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETLOADER_H ? */
