/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#define GUCEF_COMCORE_CRPCSERVERCALL_H

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
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#include "gucefCOMCORE_CRpcMetadata.h"
#define GUCEF_COMCORE_CRPCMETADATA_H
#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINTLINK_H
#include "gucefCOMCORE_CRpcEndpointLink.h"
#define GUCEF_COMCORE_CRPCENDPOINTLINK_H
#endif /* GUCEF_COMCORE_CRPCENDPOINTLINK_H ? */

#ifndef GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#include "gucefCOMCORE_CRpcDurableRequestTable.h"
#define GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#endif /* GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H ? */

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#include "gucefCOMCORE_RpcEventData.h"
#define GUCEF_COMCORE_RPCEVENTDATA_H
#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CIRpcServiceImpl;
class CRpcServerCall;
typedef CORE::CTSharedPtr< CRpcServerCall, MT::CMutex > CRpcServerCallPtr;

/*-------------------------------------------------------------------------*/

/**
 *  One call as the provider sees it (gucefCOMCORE RPC design 10.6). Given to the handler; a handler that answers
 *  later keeps GetSharedPtr() and returns RPC_HANDLER_DEFERRED.
 *
 *  Every member may be called from any thread: answers are handed to the endpoint, which sends them under its lock
 *  (the transport's SendFrame() takes a leaf lock only). Events (client stream messages, half close, cancellation,
 *  writable) are raised on the thread of the call's execution mode: the protocol thread for CALLING_THREAD, the
 *  notification pulse generator of the endpoint otherwise. An event of a call nobody subscribed to goes nowhere: the
 *  call is not linked to its event thread for it (a linked call observes that pulse generator and asks it for
 *  periodic pulses while it lives).
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcServerCall : public CORE::CTSGNotifier ,
                                                public CORE::CTSharedObjCreator< CRpcServerCall, MT::CMutex >
{
    public:

    static const CORE::CEvent MessageEvent;         /**< CRpcStreamMessageEventData: a message of the caller's stream */
    static const CORE::CEvent HalfClosedEvent;      /**< the caller sent its last stream message */
    static const CORE::CEvent CancelledEvent;       /**< CRpcStatus: cancelled by the caller, a deadline or the session */
    static const CORE::CEvent WritableEvent;        /**< Write() accepts again */

    static void RegisterEvents( void );

    virtual ~CRpcServerCall() GUCEF_VIRTUAL_OVERRIDE;

    const CRpcMethodDescriptor& GetMethod( void ) const;
    const CRpcContractPtr& GetContract( void ) const;
    const CRpcMessage& GetRequest( void ) const;           /**< the first message; empty for NO_MESSAGE streams */
    bool HasRequest( void ) const;
    const CRpcPrincipal& GetPrincipal( void ) const;
    const CRpcMetadata& GetMetadata( void ) const;
    UInt64 GetDeadline( void ) const;                       /**< Unix ms, 0 = none */
    bool IsCancelled( void ) const;
    CRpcStatus GetCancelStatus( void ) const;
    bool HasResumeFromIndex( void ) const;
    UInt64 GetResumeFromIndex( void ) const;
    UInt64 GetCallRef( void ) const;
    bool IsDone( void ) const;
    bool IsAccepted( void ) const;
    const CRpcRequestId& GetRequestId( void ) const;
    CRpcServerCallPtr GetSharedPtr( void ) const;

    /** An empty message of the method's response type */
    CRpcMessage NewResponse( void ) const;

    /** Unary and client streams: the answer. False when the call already ended */
    bool Reply( const CRpcMessage& response );

    /** Ends the call with an error status (or ends a server / bidi stream with any status) */
    bool Fail( const CRpcStatus& status );

    /** Server and bidi streams: the end of the stream (RESPONSE without payload) */
    bool Finish( const CRpcStatus& status = CRpcStatus::Ok );

    /** mayAnswerAsync methods: answers ACCEPTED; the result follows with CompleteResult() */
    bool Accept( const CRpcRequestId& id, bool durable, UInt32 etaMs, UInt32 retryAfterMs = 0 );
    bool CompleteResult( const CRpcMessage& response );
    bool CompleteResult( const CRpcStatus& status );

    /** Progress of accepted work, as STATUS answers it */
    void SetProgress( UInt32 permille, UInt32 etaMs, const CORE::CString& note );

    /** Server and bidi streams: false without credit or after the end (WritableEvent follows) */
    bool Write( const CRpcMessage& message );

    /** Client and bidi streams: credit for the caller's messages */
    void SetManualCredit( bool manual );
    void GrantCredit( UInt32 messages );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Notes that the call has an observer: only then are its events raised */
    virtual bool Subscribe( CORE::CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CRpcEndpoint;
    friend class CRpcCallTaskConsumer;
    friend class CRpcReflectionService;
    friend class CORE::CTSharedObjCreator< CRpcServerCall, MT::CMutex >;

    /**
     *  The call is made under the endpoint lock: it is linked to the pulse generator of its event thread later, by
     *  AttachEventThread() at its first event, outside that lock, as linking subscribes to the pulse generator, whose
     *  thread may be in a handler that waits for the endpoint lock. A call without events is never linked
     */
    CRpcServerCall( const CORE::PulseGeneratorPtr& eventPulseGenerator );
    void AttachEventThread( void );
    CORE::PulseGeneratorPtr GetEventPulseGenerator( void ) const { return m_eventPulseGenerator; }
    CRpcServerCall( const CRpcServerCall& src );                /**< not implemented */
    CRpcServerCall& operator=( const CRpcServerCall& src );     /**< not implemented */

    void RunHandler( void );

    /**
     *  Events for the handler: held until the handler ran, so that it sees every one of them (10.6). Raised outside
     *  the endpoint lock and without this call's lock: the thread that delivers the call's mail takes both
     */
    void RaiseCallEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL );
    void PostCallEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void EnsureInternalSubscription( void );

    struct SHeldEvent
    {
        CORE::CEvent eventId;
        CORE::CICloneable* eventData;
    };
    bool CompleteWithoutEndpoint( const CRpcStatus& status, const CRpcMessage* response );
    void OnInternalMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInternalDispatch( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    static const CORE::CEvent InternalMessageEvent;
    static const CORE::CEvent InternalDispatchEvent;

    mutable MT::CMutex m_lock;
    CRpcEndpointLinkPtr m_link;
    UInt32 m_sessionGeneration;
    UInt64 m_callRef;
    CRpcContractPtr m_contract;
    const CRpcMethodDescriptor* m_method;
    CIRpcServiceImpl* m_impl;
    CRpcMessage m_request;
    bool m_hasRequest;
    CRpcPrincipal m_principal;
    CRpcMetadata m_metadata;
    UInt64 m_deadlineUnixMs;
    bool m_hasResumeFrom;
    UInt64 m_resumeFromIndex;
    bool m_cancelled;
    CRpcStatus m_cancelStatus;
    bool m_done;
    bool m_accepted;
    bool m_durable;
    CRpcRequestId m_requestId;
    CRpcDurableRequestTablePtr m_durableTable;  /**< DURABLE: the result goes here even after the session ended */
    bool m_halfClosed;
    bool m_manualCredit;
    TRpcExecutionMode m_mode;
    UInt64 m_sendCreditLimit;       /**< provider -> caller stream messages the caller allows in total */
    UInt64 m_sentMessages;
    UInt64 m_receivedMessages;      /**< caller -> provider */
    UInt64 m_consumedMessages;
    UInt64 m_grantedLimit;
    UInt32 m_initialCredit;
    UInt64 m_deadlineMs;            /**< monotonic, 0 = none */
    CORE::CObserver m_selfObserver;
    bool m_selfSubscribed;          /**< the subscription to its own internal events, made at the first one (outside the endpoint lock) */
    CORE::PulseGeneratorPtr m_eventPulseGenerator;
    bool m_eventThreadAttached;
    bool m_observed;                /**< an observer other than the call itself subscribed, see Subscribe() */
    bool m_handlerRan;
    GUCEF::vector< SHeldEvent > m_heldEvents;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */
