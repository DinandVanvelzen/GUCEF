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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#include "gucefCOMCORE_CIRpcServiceImpl.h"
#define GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CIRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#include "gucefCOMCORE_CRpcServerCall.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRpcServerCall::MessageEvent = "GUCEF::COMCORE::CRpcServerCall::MessageEvent";
const CORE::CEvent CRpcServerCall::HalfClosedEvent = "GUCEF::COMCORE::CRpcServerCall::HalfClosedEvent";
const CORE::CEvent CRpcServerCall::CancelledEvent = "GUCEF::COMCORE::CRpcServerCall::CancelledEvent";
const CORE::CEvent CRpcServerCall::WritableEvent = "GUCEF::COMCORE::CRpcServerCall::WritableEvent";
const CORE::CEvent CRpcServerCall::InternalMessageEvent = "GUCEF::COMCORE::CRpcServerCall::InternalMessageEvent";
const CORE::CEvent CRpcServerCall::InternalDispatchEvent = "GUCEF::COMCORE::CRpcServerCall::InternalDispatchEvent";

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::RegisterEvents( void )
{GUCEF_TRACE;

    MessageEvent.Initialize();
    HalfClosedEvent.Initialize();
    CancelledEvent.Initialize();
    WritableEvent.Initialize();
    InternalMessageEvent.Initialize();
    InternalDispatchEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRpcServerCall::CRpcServerCall( const CORE::PulseGeneratorPtr& eventPulseGenerator )
    : CORE::CTSGNotifier( CORE::PulseGeneratorPtr(), true, false )
    , CORE::CTSharedObjCreator< CRpcServerCall, MT::CMutex >( this )
    , m_lock()
    , m_link()
    , m_sessionGeneration( 0 )
    , m_callRef( 0 )
    , m_contract()
    , m_method( GUCEF_NULL )
    , m_impl( GUCEF_NULL )
    , m_request()
    , m_hasRequest( false )
    , m_principal()
    , m_metadata()
    , m_deadlineUnixMs( 0 )
    , m_hasResumeFrom( false )
    , m_resumeFromIndex( 0 )
    , m_cancelled( false )
    , m_cancelStatus()
    , m_done( false )
    , m_accepted( false )
    , m_durable( false )
    , m_requestId()
    , m_durableTable()
    , m_halfClosed( false )
    , m_manualCredit( false )
    , m_mode( RPC_EXECUTION_CALLING_THREAD )
    , m_sendCreditLimit( 0 )
    , m_sentMessages( 0 )
    , m_receivedMessages( 0 )
    , m_consumedMessages( 0 )
    , m_grantedLimit( 0 )
    , m_initialCredit( 0 )
    , m_deadlineMs( 0 )
    , m_selfObserver()
    , m_selfSubscribed( false )
    , m_eventPulseGenerator( eventPulseGenerator )
    , m_eventThreadAttached( false )
    , m_observed( false )
    , m_handlerRan( false )
    , m_heldEvents()
{GUCEF_TRACE;

    // no subscription here: calls are made under the endpoint lock, and subscribing takes the lock this call's mail is
    // delivered under, by a thread whose handlers take the endpoint lock (EnsureInternalSubscription())
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CRpcServerCall::~CRpcServerCall()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_heldEvents.size(); ++i )
        GUCEF_DELETE m_heldEvents[ i ].eventData;
    m_heldEvents.clear();

    m_selfObserver.UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::CompleteWithoutEndpoint( const CRpcStatus& status, const CRpcMessage* response )
{GUCEF_TRACE;

    // the endpoint is gone: a DURABLE result is still kept for the caller's next session (6.7), anything else has
    // nobody to go to
    CRpcDurableRequestTablePtr table;
    CRpcRequestId id;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_done || !m_accepted || !m_durable || m_durableTable.IsNULL() )
            return false;
        m_done = true;
        table = m_durableTable;
        id = m_requestId;
    }
    CORE::CDynamicBuffer payload;
    CRpcStatus finalStatus = status;
    if ( finalStatus.IsOk() && GUCEF_NULL != response && !response->Encode( payload ) )
        finalStatus.Set( RPC_STATUS_INTERNAL, "the response cannot be encoded" );
    return table->Complete( id, finalStatus, &payload );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcServerCall::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcServerCall";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::RunHandler( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_impl || IsDone() )
        return;
    TRpcHandlerResult result = RPC_HANDLER_DONE;
    try
    {
        result = m_impl->OnCall( *this );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcServerCall: the handler of " << m_method->GetPath() << " threw an exception" );
        Fail( CRpcStatus( RPC_STATUS_INTERNAL, "the handler failed" ) );
        return;
    }
    if ( RPC_HANDLER_DONE == result && !IsDone() && !IsAccepted() )
        Fail( CRpcStatus( RPC_STATUS_UNKNOWN, "the handler returned without an answer" ) );

    // what arrived before the handler subscribed follows now, in order and before anything later: events raised
    // meanwhile are held as well until none is left, so nothing overtakes them. Posted without this call's lock
    while ( true )
    {
        GUCEF::vector< SHeldEvent > held;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_heldEvents.empty() )
            {
                m_handlerRan = true;
                return;
            }
            held.swap( m_heldEvents );
        }
        for ( UInt32 i=0; i<held.size(); ++i )
        {
            PostCallEvent( held[ i ].eventId, held[ i ].eventData );
            GUCEF_DELETE held[ i ].eventData;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Subscribe( CORE::CObserver* observer )
{GUCEF_TRACE;

    if ( observer != &m_selfObserver )
    {
        MT::CScopeMutex lock( m_lock );
        m_observed = true;
    }
    return CORE::CTSGNotifier::Subscribe( observer );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback )
{GUCEF_TRACE;

    if ( observer != &m_selfObserver )
    {
        MT::CScopeMutex lock( m_lock );
        m_observed = true;
    }
    return CORE::CTSGNotifier::Subscribe( observer, eventid, callback );
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::RaiseCallEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_handlerRan )
        {
            SHeldEvent held;
            held.eventId = eventId;
            held.eventData = GUCEF_NULL != eventData ? eventData->Clone() : GUCEF_NULL;
            m_heldEvents.push_back( held );
            return;
        }
    }
    PostCallEvent( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::PostCallEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // an event nobody can get goes nowhere, and the call is not linked to its event thread for it: thousands of calls
    // cancelled at once (a session that ends with them outstanding) would all become observers of that pulse
    // generator that ask it for periodic pulses. A handler that wants the events subscribes inside OnCall(), before
    // any of them is posted (they are held until it returned)
    if ( InternalMessageEvent != eventId )
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_observed )
            return;
    }

    // linked to its event thread at its first event: a call without events (most unary calls) never subscribes to a
    // pulse generator
    AttachEventThread();
    if ( InternalMessageEvent == eventId )
        EnsureInternalSubscription();
    NotifyObserversFromThread( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::AttachEventThread( void )
{GUCEF_TRACE;

    // once: mail posted before the link is made waits in the mailbox for it
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_eventThreadAttached )
            return;
        m_eventThreadAttached = true;
    }
    SetPulseGenerator( m_eventPulseGenerator );
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::EnsureInternalSubscription( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_selfSubscribed )
            return;
    }

    // client stream messages: raised as MessageEvent on the event thread, then counted as consumed for the credit.
    // Two threads may both get here: a second subscription of the same functor replaces the first
    typedef CORE::CTEventHandlerFunctor< CRpcServerCall > TEventCallback;
    TEventCallback messageCallback( this, &CRpcServerCall::OnInternalMessage );
    m_selfObserver.SubscribeTo( this, InternalMessageEvent, messageCallback );

    MT::CScopeMutex lock( m_lock );
    m_selfSubscribed = true;
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::OnInternalMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( MessageEvent, eventData );
    bool manual = false;
    {
        MT::CScopeMutex lock( m_lock );
        manual = m_manualCredit;
    }
    if ( !manual )
    {
        CRpcEndpointLink::CScope scope( m_link );
        if ( GUCEF_NULL != scope.GetEndpoint() )
            scope.GetEndpoint()->ServerConsumed( *this, 1, false );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::OnInternalDispatch( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RunHandler();
}

/*-------------------------------------------------------------------------*/

const CRpcMethodDescriptor&
CRpcServerCall::GetMethod( void ) const
{GUCEF_TRACE;

    return *m_method;
}

/*-------------------------------------------------------------------------*/

const CRpcContractPtr&
CRpcServerCall::GetContract( void ) const
{GUCEF_TRACE;

    return m_contract;
}

/*-------------------------------------------------------------------------*/

const CRpcMessage&
CRpcServerCall::GetRequest( void ) const
{GUCEF_TRACE;

    return m_request;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::HasRequest( void ) const
{GUCEF_TRACE;

    return m_hasRequest;
}

/*-------------------------------------------------------------------------*/

const CRpcPrincipal&
CRpcServerCall::GetPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

const CRpcMetadata&
CRpcServerCall::GetMetadata( void ) const
{GUCEF_TRACE;

    return m_metadata;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcServerCall::GetDeadline( void ) const
{GUCEF_TRACE;

    return m_deadlineUnixMs;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::IsCancelled( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_cancelled;
}

/*-------------------------------------------------------------------------*/

CRpcStatus
CRpcServerCall::GetCancelStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_cancelStatus;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::HasResumeFromIndex( void ) const
{GUCEF_TRACE;

    return m_hasResumeFrom;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcServerCall::GetResumeFromIndex( void ) const
{GUCEF_TRACE;

    return m_resumeFromIndex;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcServerCall::GetCallRef( void ) const
{GUCEF_TRACE;

    return m_callRef;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::IsDone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_done;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::IsAccepted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_accepted;
}

/*-------------------------------------------------------------------------*/

const CRpcRequestId&
CRpcServerCall::GetRequestId( void ) const
{GUCEF_TRACE;

    return m_requestId;
}

/*-------------------------------------------------------------------------*/

CRpcServerCallPtr
CRpcServerCall::GetSharedPtr( void ) const
{GUCEF_TRACE;

    return CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CRpcMessage
CRpcServerCall::NewResponse( void ) const
{GUCEF_TRACE;

    return CRpcMessage( m_contract, m_method->GetResponseType() );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Reply( const CRpcMessage& response )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return false;
    if ( IsAccepted() )
        return scope.GetEndpoint()->ServerCompleteResult( *this, CRpcStatus::Ok, &response );
    return scope.GetEndpoint()->ServerAnswer( *this, GUCEF_RPC_FRAME_RESPONSE, CRpcStatus::Ok, &response );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Fail( const CRpcStatus& status )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return CompleteWithoutEndpoint( status, GUCEF_NULL );
    if ( IsAccepted() )
        return scope.GetEndpoint()->ServerCompleteResult( *this, status, GUCEF_NULL );
    return scope.GetEndpoint()->ServerAnswer( *this, GUCEF_RPC_FRAME_RESPONSE, status, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Finish( const CRpcStatus& status )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return false;
    return scope.GetEndpoint()->ServerAnswer( *this, GUCEF_RPC_FRAME_RESPONSE, status, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Accept( const CRpcRequestId& id, bool durable, UInt32 etaMs, UInt32 retryAfterMs )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return false;
    return scope.GetEndpoint()->ServerAccept( *this, id, durable, etaMs, retryAfterMs );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::CompleteResult( const CRpcMessage& response )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return CompleteWithoutEndpoint( CRpcStatus::Ok, &response );
    return scope.GetEndpoint()->ServerCompleteResult( *this, CRpcStatus::Ok, &response );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::CompleteResult( const CRpcStatus& status )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return CompleteWithoutEndpoint( status, GUCEF_NULL );
    return scope.GetEndpoint()->ServerCompleteResult( *this, status, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::SetProgress( UInt32 permille, UInt32 etaMs, const CORE::CString& note )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL != scope.GetEndpoint() )
        scope.GetEndpoint()->ServerSetProgress( *this, permille, etaMs, note );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServerCall::Write( const CRpcMessage& message )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return false;
    return scope.GetEndpoint()->ServerWrite( *this, message );
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::SetManualCredit( bool manual )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_manualCredit = manual;
}

/*-------------------------------------------------------------------------*/

void
CRpcServerCall::GrantCredit( UInt32 messages )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL != scope.GetEndpoint() )
        scope.GetEndpoint()->ServerConsumed( *this, messages, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
