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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H
#define GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H

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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERHOSTINFO_H
#include "gucefDRNUTILS_CTaskManagerHostInfo.h"
#define GUCEF_DRNUTILS_CTASKMANAGERHOSTINFO_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERHOSTINFO_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCONTROLCOMMAND_H
#include "gucefDRNUTILS_CTaskManagerControlCommand.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCONTROLCOMMAND_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCONTROLCOMMAND_H ? */

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
 *  Client facade of gucef.drnutils.tasks 1.0 over an RPC endpoint: info, control commands and viewing mode. Keeping a
 *  Watch stream open (StartViewing()) puts the host in viewing mode, so its task tables are sampled and published while
 *  a form shows them. Events are raised on the thread of the given pulse generator; nothing blocks.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CTaskManagerClient : public CORE::CTSGNotifier
{
    public:

    static const CORE::CString ClassTypeName;

    static const CORE::CEvent InfoReceivedEvent;            /**< TInfoReceivedEventData */
    static const CORE::CEvent CommandCompletedEvent;        /**< TCommandCompletedEventData */
    static const CORE::CEvent ViewingChangedEvent;          /**< no data: IsViewing() changed */

    typedef CORE::CTCloneableObj< STaskManagerHostInfo >        TInfoReceivedEventData;
    typedef CORE::CTCloneableObj< SDrnUtilsCommandOutcome >     TCommandCompletedEventData;

    static void RegisterEvents( void );

    explicit CTaskManagerClient( const CORE::PulseGeneratorPtr& eventPulseGenerator );

    virtual ~CTaskManagerClient() GUCEF_VIRTUAL_OVERRIDE;

    void SetEndpoint( const COMCORE::CRpcEndpointPtr& endpoint );

    COMCORE::CRpcEndpointPtr GetEndpoint( void ) const;

    bool IsConnected( void ) const;

    virtual bool RequestInfo( void );

    const STaskManagerHostInfo& GetLastInfo( void ) const;

    /** TaskManager/Control; returns the command id (empty when not sent) */
    virtual CORE::CString Control( const STaskManagerControlCommand& command );

    CORE::CString ResizePool( const CORE::CString& poolName, UInt32 minWorkers, Int32 maxThreads, const CORE::CString& reason );
    CORE::CString CancelTask( UInt32 taskId, const CORE::CString& reason );
    CORE::CString StopTask( UInt32 taskId, const CORE::CString& reason );

    /** Opens a TaskManager/Watch stream with the given sample interval (at least 1000 ms) */
    virtual bool StartViewing( UInt32 sampleIntervalMs, bool includeThreads, bool includeTasks );

    virtual void StopViewing( void );

    bool IsViewing( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTaskManagerClient( const CTaskManagerClient& src );                /**< not implemented */
    CTaskManagerClient& operator=( const CTaskManagerClient& src );     /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CTaskManagerClient > TEventCallback;
    enum ECallKind { CALL_INFO = 0, CALL_COMMAND = 1 };
    struct SPendingCall
    {
        Int32 kind;
        CORE::CString commandId;
        CORE::CFutureResult* future;
    };
    typedef GUCEF::vector< SPendingCall > TPendingCallVector;

    bool Send( const CORE::CString& methodName, const COMCORE::CRpcMessage& request, Int32 kind, const CORE::CString& commandId );
    bool NewRequest( const CORE::CString& methodName, COMCORE::CRpcMessage& request ) const;
    void OnPollTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEndpointEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void CheckConnection( void );
    void DropPendingCalls( void );
    void OnStreamEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void EndViewing( void );

    COMCORE::CRpcEndpointPtr m_endpoint;
    STaskManagerHostInfo m_lastInfo;
    TPendingCallVector m_pending;       /**< the thread of the event pulse generator only */
    CORE::CTimer* m_pollTimer;          /**< enabled only while calls are pending */
    bool m_connected;
    COMCORE::CIRpcStreamPtr m_watch;
    UInt64 m_samplesReceived;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H ? */
