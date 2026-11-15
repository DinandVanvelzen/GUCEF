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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERHOST_H
#define GUCEF_DRNUTILS_CTASKMANAGERHOST_H

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

#ifndef GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#include "gucefDRNUTILS_CITaskManagerSource.h"
#define GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERHOSTSETTINGS_H
#include "gucefDRNUTILS_CTaskManagerHostSettings.h"
#define GUCEF_DRNUTILS_CTASKMANAGERHOSTSETTINGS_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERHOSTSETTINGS_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERHOSTINFO_H
#include "gucefDRNUTILS_CTaskManagerHostInfo.h"
#define GUCEF_DRNUTILS_CTASKMANAGERHOSTINFO_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERHOSTINFO_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCONTROLCOMMAND_H
#include "gucefDRNUTILS_CTaskManagerControlCommand.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCONTROLCOMMAND_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCONTROLCOMMAND_H ? */

#ifndef GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#include "gucefDRNUTILS_CIDrnUtilsAuditSink.h"
#define GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#endif /* GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H
#include "gucefDRNUTILS_CDrnUtilsLogAuditSink.h"
#define GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H ? */

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

class CTaskManagerWatchStream;
class CTaskManagerSample;
class CTaskManagerSampler;
class CTaskManagerTablePublisher;
class CCommandIdempotencyCache;
class CControlRateLimiter;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Serves the task manager view remotely: the service TaskManager of gucef.drnutils.tasks 1.0, the sampler (only while
 *  somebody watches: an open Watch stream puts the host in viewing mode) and the pools, threads, tasks and scheduled
 *  jobs grids. Created, linked and destroyed on the service's notification thread.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CTaskManagerHost : public CORE::CTSGNotifier
{
    public:

    static const CORE::CString ClassTypeName;

    CTaskManagerHost( CITaskManagerSource& source                              ,
                      const CTaskManagerHostSettings& settings                 ,
                      const CORE::PulseGeneratorPtr& notificationPulseGenerator );

    virtual ~CTaskManagerHost() GUCEF_VIRTUAL_OVERRIDE;

    const CTaskManagerHostSettings& GetSettings( void ) const;

    /** Not owned; NULL restores the default (CDrnUtilsLogAuditSink) */
    void SetAuditSink( CIDrnUtilsAuditSink* sink );

    bool Expose( const COMCORE::CRpcServiceSetPtr& services, CORE::CString& error );

    void Withdraw( void );

    bool IsExposed( void ) const;

    bool PublishTables( const CORE::CString& drnSessionName, CORE::CString& error );

    void StopPublishing( void );

    /** Whether a Watch stream is open (or was within ViewingGraceMs): the sampler runs only then */
    bool IsInViewingMode( void ) const;

    /**
     *  What the open Watch streams were sent: samples written, and samples that were replaced by a later one while the
     *  viewer had no credit (the latest sample wins, design 5.3). On the thread of the event pulse generator
     */
    void GetWatchStatistics( UInt32& openStreams, UInt64& samplesWritten, UInt64& samplesReplaced ) const;

    /** Table name ("pools", "threads", "tasks", "scheduledjobs") to grid name */
    void GetTableNames( CORE::CString::StringMap& tableNames ) const;

    void GetHostInfo( const COMCORE::CRpcPrincipal& principal, STaskManagerHostInfo& info ) const;

    /** The control path: roles, controllable pools and task types, bounds, rate limit, idempotency, audit, then the source */
    TDrnUtilsCommandStatus Control( const COMCORE::CRpcPrincipal& principal        ,
                                    const CORE::CString& commandId                 ,
                                    const STaskManagerControlCommand& command      ,
                                    CORE::CString::StringVector& messages          ,
                                    bool& replayed                                 );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTaskManagerHost( const CTaskManagerHost& src );                /**< not implemented */
    CTaskManagerHost& operator=( const CTaskManagerHost& src );     /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CTaskManagerHost > TEventCallback;
    typedef GUCEF::vector< CTaskManagerWatchStream* > TWatchStreamVector;

    COMCORE::TRpcHandlerResult OnGetInfo( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListPools( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListThreads( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListTasks( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnWatch( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnControl( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnListScheduledJobs( COMCORE::CRpcServerCall& call );

    void OnSampleReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCallWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCallCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnReleaseTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ReleaseLater( const COMCORE::CRpcServerCallPtr& call );
    void OnGraceTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    bool CheckViewAccess( const COMCORE::CRpcPrincipal& principal, CORE::CString& reason ) const;
    bool IsProtectedPool( const CORE::CString& poolName ) const;
    void Audit( const SDrnUtilsAuditRecord& record );
    void UpdateSampler( void );
    void EndViewing( void );
    void RemoveStream( CORE::CNotifier* callNotifier );
    void RemoveAllStreams( const COMCORE::CRpcStatus& status );
    void BuildSampleMessage( const CTaskManagerWatchStream& stream, COMCORE::CRpcMessage& message ) const;
    void GetListSample( bool includeThreads, bool includeTasks, bool includeScheduledJobs, CTaskManagerSample& sample );

    CITaskManagerSource* m_source;
    CTaskManagerHostSettings m_settings;
    CDrnUtilsLogAuditSink m_defaultAuditSink;
    CIDrnUtilsAuditSink* m_auditSink;
    mutable MT::CMutex m_lock;                      /**< a leaf: the audit sink, the viewing flag */
    COMCORE::CRpcServiceImpl* m_service;
    COMCORE::CRpcServiceSetPtr m_services;
    COMCORE::CRpcContractPtr m_contract;
    CCommandIdempotencyCache* m_commands;
    CControlRateLimiter* m_rateLimiter;
    CTaskManagerSampler* m_sampler;
    CTaskManagerTablePublisher* m_tablePublisher;
    CTaskManagerSample* m_current;                  /**< the latest sample, tasks and threads of the latest sample that had them */
    CTaskManagerSample* m_listPrevious;             /**< for the rates of list answers outside viewing mode */
    TWatchStreamVector m_streams;                   /**< notification thread only */
    GUCEF::vector< COMCORE::CRpcServerCallPtr > m_releasedCalls;    /**< let go of at the next pulse: never inside an event of the call itself */
    CORE::CTimer* m_releaseTimer;
    CORE::CTimer* m_graceTimer;
    bool m_viewing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERHOST_H ? */
