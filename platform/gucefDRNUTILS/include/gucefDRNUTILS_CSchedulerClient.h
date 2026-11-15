/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#define GUCEF_DRNUTILS_CSCHEDULERCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLCOMMAND_H
#include "gucefCORE_CJobControlCommand.h"
#define GUCEF_CORE_CJOBCONTROLCOMMAND_H
#endif /* GUCEF_CORE_CJOBCONTROLCOMMAND_H ? */

#ifndef GUCEF_CORE_CJOBSUBMISSION_H
#include "gucefCORE_CJobSubmission.h"
#define GUCEF_CORE_CJOBSUBMISSION_H
#endif /* GUCEF_CORE_CJOBSUBMISSION_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H
#include "gucefDRNUTILS_CSchedulerHostInfo.h"
#define GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERJOBDETAILANSWER_H
#include "gucefDRNUTILS_CSchedulerJobDetailAnswer.h"
#define GUCEF_DRNUTILS_CSCHEDULERJOBDETAILANSWER_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERJOBDETAILANSWER_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSCOMMANDOUTCOME_H
#include "gucefDRNUTILS_CDrnUtilsCommandOutcome.h"
#define GUCEF_DRNUTILS_CDRNUTILSCOMMANDOUTCOME_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSCOMMANDOUTCOME_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Client facade of gucef.drnutils.scheduler 1.0 over an RPC endpoint, for forms and tools: it builds the requests
 *  (with a fresh command id for every command), calls asynchronously and raises the outcome as events on the thread of
 *  the given pulse generator (the GUI thread). Nothing blocks. Times are UTC; translation to a display zone is the GUI's.
 *  The request methods are virtual so that form tests can substitute a recording fake.
 *  Pause and resume are not in this version.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CSchedulerClient : public CORE::CTSGNotifier
{
    public:

    static const CORE::CString ClassTypeName;

    static const CORE::CEvent InfoReceivedEvent;            /**< TInfoReceivedEventData */
    static const CORE::CEvent CommandCompletedEvent;        /**< TCommandCompletedEventData */
    static const CORE::CEvent ConnectionChangedEvent;       /**< no data: IsConnected() changed */
    static const CORE::CEvent JobDetailReceivedEvent;       /**< TJobDetailReceivedEventData */

    typedef CORE::CTCloneableObj< SSchedulerHostInfo >          TInfoReceivedEventData;
    typedef CORE::CTCloneableObj< SDrnUtilsCommandOutcome >     TCommandCompletedEventData;
    typedef CORE::CTCloneableObj< SSchedulerJobDetailAnswer >   TJobDetailReceivedEventData;

    static void RegisterEvents( void );

    explicit CSchedulerClient( const CORE::PulseGeneratorPtr& eventPulseGenerator );

    virtual ~CSchedulerClient() GUCEF_VIRTUAL_OVERRIDE;

    /** The endpoint of the session with the node; NULL disconnects */
    void SetEndpoint( const COMCORE::CRpcEndpointPtr& endpoint );

    COMCORE::CRpcEndpointPtr GetEndpoint( void ) const;

    bool IsConnected( void ) const;

    /** Scheduler/GetInfo; the answer arrives as InfoReceivedEvent. False when it could not be sent */
    virtual bool RequestInfo( void );

    /** The last answer of GetInfo */
    const SSchedulerHostInfo& GetLastInfo( void ) const;

    /** Scheduler/Control; returns the command id (empty when not sent); the outcome arrives as CommandCompletedEvent */
    virtual CORE::CString Control( const CORE::SJobControlCommand& command, bool waitForRun = false );

    /** Convenience forms of Control() */
    CORE::CString RunNow( const CORE::CString& jobId, const CORE::CString& reason, bool waitForRun = false );
    CORE::CString CancelRun( UInt64 runId, const CORE::CString& reason );
    CORE::CString SetJobEnabled( const CORE::CString& jobId, bool enabled, const CORE::CString& reason );

    /** Scheduler/SubmitJob: a job as data for a task type the node allows; returns the command id */
    virtual CORE::CString SubmitJob( const CORE::SJobSubmission& submission );

    /**
     *  Scheduler/GetJob: the detail of one job (its row with the actions this viewer may use, its recent runs, with
     *  includeSourceStatus the status of its work source, its description and an exposed payload). Returns a request id
     *  (empty when not sent); the answer arrives as JobDetailReceivedEvent carrying that id, also when the node refused
     */
    virtual CORE::CString RequestJobDetail( const CORE::CString& jobId, UInt32 recentRuns = 10, bool includeSourceStatus = false );

    /** A new random command id (128 bit, hex) */
    static CORE::CString NewCommandId( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSchedulerClient( const CSchedulerClient& src );                /**< not implemented */
    CSchedulerClient& operator=( const CSchedulerClient& src );     /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CSchedulerClient > TEventCallback;
    enum ECallKind { CALL_INFO = 0, CALL_COMMAND = 1, CALL_JOBDETAIL = 2 };
    struct SPendingCall
    {
        Int32 kind;
        CORE::CString commandId;        /**< the command id, or the request id of a job detail */
        CORE::CString jobId;            /**< CALL_JOBDETAIL */
        CORE::CFutureResult* future;
    };
    typedef GUCEF::vector< SPendingCall > TPendingCallVector;

    bool Send( const CORE::CString& methodName, const COMCORE::CRpcMessage& request, Int32 kind, const CORE::CString& commandId,
               const CORE::CString& jobId = CORE::CString::Empty );
    void OnJobDetailAnswer( const SPendingCall& call, const COMCORE::CRpcCallResult& result );
    bool NewRequest( const CORE::CString& methodName, COMCORE::CRpcMessage& request ) const;
    void OnPollTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEndpointEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void CheckConnection( void );
    void DropPendingCalls( void );

    COMCORE::CRpcEndpointPtr m_endpoint;
    SSchedulerHostInfo m_lastInfo;
    TPendingCallVector m_pending;       /**< the thread of the event pulse generator only */
    CORE::CTimer* m_pollTimer;          /**< enabled only while calls are pending */
    bool m_connected;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERCLIENT_H ? */
