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

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOST_H
#define GUCEF_DRNUTILS_CSCHEDULERHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICESET_H
#include "gucefCOMCORE_CRpcServiceSet.h"
#define GUCEF_COMCORE_CRPCSERVICESET_H
#endif /* GUCEF_COMCORE_CRPCSERVICESET_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H
#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOSTSETTINGS_H
#include "gucefDRNUTILS_CSchedulerHostSettings.h"
#define GUCEF_DRNUTILS_CSCHEDULERHOSTSETTINGS_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERHOSTSETTINGS_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H
#include "gucefDRNUTILS_CSchedulerHostInfo.h"
#define GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H ? */

#ifndef GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#include "gucefDRNUTILS_CIDrnUtilsAuditSink.h"
#define GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#endif /* GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H
#include "gucefDRNUTILS_CDrnUtilsLogAuditSink.h"
#define GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H
#include "gucefDRNUTILS_CISchedulerCustomActionHandler.h"
#define GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

class CSchedulerWatchStream;
class CSchedulerChangeJournal;
class CCommandIdempotencyCache;
class CControlRateLimiter;
class CSchedulerTablePublisher;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Serves one scheduler remotely: the service Scheduler of gucef.drnutils.scheduler 1.0 (registered on a service set in
 *  RPC_EXECUTION_NOTIFICATION_THREAD mode), the watch streams, and the jobs, lanes and runs grids in a DRN session.
 *  The control path (Control(), SubmitJob()) is shared with local admin commands and application contracts: roles,
 *  allowed actions, secure transport, rate limit, idempotency by command id, audit, then the source.
 *  Created, linked and destroyed on the service's notification thread (the thread of the pulse generator given).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CSchedulerHost : public CORE::CTSGNotifier
{
    public:

    static const CORE::CString ClassTypeName;

    CSchedulerHost( CISchedulerSource& source                              ,
                    const CSchedulerHostSettings& settings                 ,
                    const CORE::PulseGeneratorPtr& notificationPulseGenerator );

    virtual ~CSchedulerHost() GUCEF_VIRTUAL_OVERRIDE;

    const CSchedulerHostSettings& GetSettings( void ) const;

    CISchedulerSource& GetSource( void );

    /** Not owned; NULL restores the default (CDrnUtilsLogAuditSink) */
    void SetAuditSink( CIDrnUtilsAuditSink* sink );

    /** Not owned; NULL = no service level custom actions */
    void SetCustomActionHandler( CISchedulerCustomActionHandler* handler );

    /** Registers Scheduler on the set; false with the reason when the exposure is NONE */
    bool Expose( const COMCORE::CRpcServiceSetPtr& services, CORE::CString& error );

    void Withdraw( void );

    bool IsExposed( void ) const;

    /** Publishes the jobs, lanes and runs grids in the named DRN session; nothing is published before */
    bool PublishTables( const CORE::CString& drnSessionName, CORE::CString& error );

    void StopPublishing( void );

    /** Table name ("jobs", "lanes", "runs") to grid name */
    void GetTableNames( CORE::CString::StringMap& tableNames ) const;

    /** What GetInfo answers to the principal: allowed actions and submittable task types filtered by its roles */
    void GetHostInfo( const COMCORE::CRpcPrincipal& principal, SSchedulerHostInfo& info ) const;

    /** The control path shared with local admin commands and application contracts */
    TDrnUtilsCommandStatus Control( const COMCORE::CRpcPrincipal& principal       ,
                                    const CORE::CString& commandId                ,
                                    const CORE::SJobControlCommand& command       ,
                                    CORE::SJobControlResult& result               ,
                                    bool& replayed                                );

    /** Submission of a job as data: role scheduler.submit, task type allowed by the settings and the source, payload bound */
    TDrnUtilsCommandStatus SubmitJob( const COMCORE::CRpcPrincipal& principal     ,
                                      const CORE::CString& commandId              ,
                                      const CORE::SJobSubmission& submission      ,
                                      CORE::SJobControlResult& result             ,
                                      bool& replayed                              );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSchedulerHost( const CSchedulerHost& src );                /**< not implemented */
    CSchedulerHost& operator=( const CSchedulerHost& src );     /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CSchedulerHost > TEventCallback;
    typedef GUCEF::vector< CSchedulerWatchStream* > TWatchStreamVector;
    typedef GUCEF::vector< COMCORE::CRpcServerCallPtr > TServerCallVector;
    typedef GUCEF::map< UInt64, TServerCallVector > TRunWaitMap;

    struct SChange
    {
        Int32 kind;                 /**< CSchedulerChangeJournal::EKind, or 6 for the host */
        CORE::CString key;
        UInt64 revision;
        bool endedRun;
    };
    typedef GUCEF::vector< SChange > TChangeVector;

    COMCORE::TRpcHandlerResult OnGetInfo( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListJobs( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnGetJob( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListLanes( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListRuns( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnWatch( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnControl( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnSubmitJob( COMCORE::CRpcServerCall& call );

    void OnSourceChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCallWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCallCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnReleaseTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ReleaseLater( const COMCORE::CRpcServerCallPtr& call );
    void OnRunWaitCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStreamTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPublishTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void RefreshModel( void );
    void SanitizeSnapshot( CORE::CJobSchedulerSnapshot& snapshot ) const;
    bool CheckViewAccess( const COMCORE::CRpcPrincipal& principal, CORE::CString& reason ) const;
    bool IsSecure( const COMCORE::CRpcPrincipal& principal ) const;
    TDrnUtilsCommandStatus CheckControlAccess( const COMCORE::CRpcPrincipal& principal, const CORE::SJobControlCommand& command,
                                               const TSchedulerActionInfoVector& declared, const CORE::CJobSchedulerSnapshot* fresh,
                                               CORE::CString& reason ) const;
    void GetDeclaredCustomActions( TSchedulerActionInfoVector& declared ) const;
    void GetAllowedActions( const COMCORE::CRpcPrincipal& principal, const CORE::SJobStatus* job,
                            const TSchedulerActionInfoVector& declared, TSchedulerActionInfoVector& actions ) const;
    void Audit( const SDrnUtilsAuditRecord& record );
    UInt64 GetMonotonicMs( void ) const;
    void FillOutcome( const CORE::SJobControlResult& result, bool replayed, COMCORE::CRpcMessage& outcome ) const;
    void FlushStream( CSchedulerWatchStream& stream );
    void BuildStreamMessages( CSchedulerWatchStream& stream, const TSchedulerActionInfoVector& declared );
    void RemoveStream( CORE::CNotifier* callNotifier );
    void RemoveAllStreams( const COMCORE::CRpcStatus& status );
    void CompleteRunWaits( const TChangeVector& changes );
    void CompleteAllRunWaits( const COMCORE::CRpcStatus& status );
    void StartPublishTimerIfNeeded( void );
    void BuildJobRow( const CORE::SJobStatus& job, const COMCORE::CRpcPrincipal& principal,
                      const TSchedulerActionInfoVector& declared, COMCORE::CRpcMessage& row ) const;

    CISchedulerSource* m_source;
    CSchedulerHostSettings m_settings;
    CDrnUtilsLogAuditSink m_defaultAuditSink;
    CIDrnUtilsAuditSink* m_auditSink;
    CISchedulerCustomActionHandler* m_customActionHandler;
    mutable MT::CMutex m_lock;                      /**< a leaf: model, journal, run waits; never held while calling out */
    COMCORE::CRpcServiceImpl* m_service;
    COMCORE::CRpcServiceSetPtr m_services;
    COMCORE::CRpcContractPtr m_contract;
    CORE::CJobSchedulerSnapshot* m_model;           /**< the last snapshot of the source, hidden jobs removed, sensitive texts redacted */
    CSchedulerChangeJournal* m_journal;
    CCommandIdempotencyCache* m_commands;
    CControlRateLimiter* m_rateLimiter;
    CSchedulerTablePublisher* m_tablePublisher;
    TWatchStreamVector m_streams;                   /**< notification thread only */
    TRunWaitMap m_runWaits;                         /**< notification thread only */
    GUCEF::vector< COMCORE::CRpcServerCallPtr > m_releasedCalls;    /**< let go of at the next pulse: never inside an event of the call itself */
    CORE::CTimer* m_releaseTimer;
    CORE::CTimer* m_streamTimer;
    CORE::CTimer* m_publishTimer;
    bool m_publishDirty;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERHOST_H ? */
