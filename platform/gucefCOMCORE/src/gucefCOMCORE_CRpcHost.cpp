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

#include <string.h>

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

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#include "gucefCOMCORE_CRpcHost.h"

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

const CORE::CEvent CRpcHost::EndpointCreatedEvent = "GUCEF::COMCORE::CRpcHost::EndpointCreatedEvent";
const CORE::CEvent CRpcHost::EndpointClosedEvent = "GUCEF::COMCORE::CRpcHost::EndpointClosedEvent";

/*-------------------------------------------------------------------------*/

void
CRpcHost::RegisterEvents( void )
{GUCEF_TRACE;

    EndpointCreatedEvent.Initialize();
    EndpointClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRpcHost::CRpcHost( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRpcHostSettings& settings )
    : CORE::CTSGNotifier( notificationPulseGenerator, true, false )
    , m_lock()
    , m_settings( settings )
    , m_services( CRpcServiceSet::Create() )
    , m_accessPolicy()
    , m_durable( CRpcDurableRequestTable::Create( settings.endpointSettings.durableResultRetentionMs, settings.endpointSettings.maxDurableRequestsPerSubject ) )
    , m_listeners()
    , m_endpoints()
    , m_pending()
    , m_released()
    , m_releasePulseGenerators()
    , m_listenerObserver()
    , m_pendingObserver()
    , m_endpointObserver()
    , m_pulseObserver()
    , m_releaseObserver()
{GUCEF_TRACE;

    RegisterEvents();
    if ( !notificationPulseGenerator.IsNULL() )
    {
        typedef CORE::CTEventHandlerFunctor< CRpcHost > TEventCallback;
        TEventCallback pulse( this, &CRpcHost::OnPulse );
        m_pulseObserver.SubscribeTo( CORE::PulseGeneratorPtr( notificationPulseGenerator ).GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
    }
}

/*-------------------------------------------------------------------------*/

CRpcHost::~CRpcHost()
{GUCEF_TRACE;

    m_pulseObserver.UnsubscribeAllFromObserver();
    Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcHost";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::Start( void )
{GUCEF_TRACE;

    bool allListening = true;
    for ( UInt32 i=0; i<m_settings.listeners.size(); ++i )
    {
        const CRpcHostSettings::SListener& entry = m_settings.listeners[ i ];
        CIRpcTransportListenerPtr listener = CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( entry.uri.GetScheme() );
        if ( listener.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcHost: no RPC transport listener of type \"" << entry.uri.GetScheme() << "\"" );
            allListening = false;
            continue;
        }
        listener->LoadConfig( entry.config );
        if ( !AddListener( listener ) || !listener->Listen( entry.uri ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcHost: cannot listen on " << entry.uri.ToUriInStringForm() );
            allListening = false;
        }
    }
    return allListening;
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::Stop( void )
{GUCEF_TRACE;

    GUCEF::vector< CIRpcTransportListenerPtr > listeners;
    GUCEF::vector< CRpcEndpointPtr > endpoints;
    TPendingVector pending;
    TReleasedVector released;
    TPulseGeneratorSet releasePulseGenerators;
    {
        MT::CScopeMutex lock( m_lock );
        listeners.swap( m_listeners );
        endpoints.swap( m_endpoints );
        pending.swap( m_pending );
        released.swap( m_released );
        releasePulseGenerators.swap( m_releasePulseGenerators );
    }
    m_listenerObserver.UnsubscribeAllFromObserver();
    m_pendingObserver.UnsubscribeAllFromObserver();
    m_endpointObserver.UnsubscribeAllFromObserver();
    m_releaseObserver.UnsubscribeAllFromObserver();
    for ( UInt32 i=0; i<released.size(); ++i )
        released[ i ].pulseGenerator->RequestStopOfPeriodicUpdates( this );
    for ( UInt32 i=0; i<listeners.size(); ++i )
        listeners[ i ]->StopListening();
    for ( UInt32 i=0; i<pending.size(); ++i )
        pending[ i ].transport->Close();
    for ( UInt32 i=0; i<endpoints.size(); ++i )
    {
        endpoints[ i ]->SetHost( GUCEF_NULL );
        endpoints[ i ]->Close( false );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::AddListener( const CIRpcTransportListenerPtr& listener )
{GUCEF_TRACE;

    if ( listener.IsNULL() )
        return false;
    typedef CORE::CTEventHandlerFunctor< CRpcHost > TEventCallback;
    TEventCallback accepted( this, &CRpcHost::OnTransportAccepted );
    CIRpcTransportListenerPtr subscribed( listener );
    m_listenerObserver.SubscribeTo( subscribed.GetPointerAlways(), CIRpcTransportListener::TransportAcceptedEvent, accepted );
    MT::CScopeMutex lock( m_lock );
    m_listeners.push_back( listener );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::OnTransportAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    AcceptTransport( static_cast< CRpcTransportAcceptedEventData* >( eventData )->transport );
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::AcceptTransport( const CIRpcTransportPtr& transport )
{GUCEF_TRACE;

    if ( transport.IsNULL() )
        return false;

    // until its HELLO the transport is the host's; the HELLO decides which endpoint gets it
    typedef CORE::CTEventHandlerFunctor< CRpcHost > TEventCallback;
    TEventCallback frame( this, &CRpcHost::OnPendingFrame );
    TEventCallback disconnected( this, &CRpcHost::OnPendingDisconnected );
    {
        MT::CScopeMutex lock( m_lock );
        SPending pending;
        pending.transport = transport;
        pending.sinceMs = CRpcDurableRequestTable::NowMs();
        m_pending.push_back( pending );
    }
    CIRpcTransportPtr subscribed( transport );
    m_pendingObserver.SubscribeTo( subscribed.GetPointerAlways(), CIRpcTransport::FrameReceivedEvent, frame );
    m_pendingObserver.SubscribeTo( subscribed.GetPointerAlways(), CIRpcTransport::DisconnectedEvent, disconnected );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::TakePending( CORE::CNotifier* notifier, CIRpcTransportPtr& transport )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_pending.size(); ++i )
    {
        if ( static_cast< CORE::CNotifier* >( m_pending[ i ].transport.GetPointerAlways() ) == notifier )
        {
            transport = m_pending[ i ].transport;
            m_pending.erase( m_pending.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::OnPendingDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CIRpcTransportPtr transport;
    if ( TakePending( notifier, transport ) )
    {
        m_pendingObserver.UnsubscribeFrom( *notifier );

        // the host may be its last holder: inside its own event it must stay
        ReleaseOnItsThread( transport );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::ReleaseOnItsThread( const CIRpcTransportPtr& transportRef )
{GUCEF_TRACE;

    // Called inside an event of the transport, so on its protocol thread (the thread of its pulse generator): the
    // reference goes at that pulse generator's next pulse, once the event is over. The subscription to a pulse
    // generator is made and ended only on its own thread, so they never cross
    CIRpcTransportPtr transport( transportRef );
    CORE::PulseGeneratorPtr pulseGenerator = transport->GetPulseGenerator();
    if ( pulseGenerator.IsNULL() )
        return;     // a transport always has one while it raises events; without one there is no later pulse to wait for
    bool subscribe = false;
    {
        MT::CScopeMutex lock( m_lock );
        SReleased released;
        released.transport = transport;
        released.pulseGenerator = pulseGenerator;
        m_released.push_back( released );
        subscribe = m_releasePulseGenerators.insert( pulseGenerator.GetPointerAlways() ).second;
    }
    if ( subscribe )
    {
        typedef CORE::CTEventHandlerFunctor< CRpcHost > TEventCallback;
        TEventCallback pulse( this, &CRpcHost::OnReleasePulse );
        m_releaseObserver.SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
        pulseGenerator->RequestPeriodicPulses( this, 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::OnReleasePulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the pulse thread of the transports released on it: their events are over
    TReleasedVector released;
    CORE::PulseGeneratorPtr pulseGenerator;
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<m_released.size(); )
        {
            if ( static_cast< CORE::CNotifier* >( m_released[ i ].pulseGenerator.GetPointerAlways() ) == notifier )
            {
                pulseGenerator = m_released[ i ].pulseGenerator;
                released.push_back( m_released[ i ] );
                m_released[ i ] = m_released.back();
                m_released.pop_back();
            }
            else
                ++i;
        }
        if ( pulseGenerator.IsNULL() )
            return;
        m_releasePulseGenerators.erase( pulseGenerator.GetPointerAlways() );
    }
    m_releaseObserver.UnsubscribeFrom( *notifier );
    pulseGenerator->RequestStopOfPeriodicUpdates( this );
    released.clear();
}

/*-------------------------------------------------------------------------*/

CRpcEndpointPtr
CRpcHost::NewEndpoint( void )
{GUCEF_TRACE;

    CRpcEndpointPtr endpoint = CRpcEndpoint::Create( GetPulseGenerator(), m_settings.endpointSettings );
    endpoint->SetServiceSet( m_services );
    endpoint->SetDurableRequestTable( m_durable );
    if ( !m_accessPolicy.IsNULL() )
        endpoint->SetAccessPolicy( m_accessPolicy );
    endpoint->SetHost( this );
    typedef CORE::CTEventHandlerFunctor< CRpcHost > TEventCallback;
    TEventCallback closed( this, &CRpcHost::OnEndpointClosed );
    m_endpointObserver.SubscribeTo( endpoint.GetPointerAlways(), CRpcEndpoint::SessionClosedEvent, closed );
    return endpoint;
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::MaySessionStart( const CRpcPrincipal& principal ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 live = 0;
    UInt32 ofSubject = 0;
    for ( UInt32 i=0; i<m_endpoints.size(); ++i )
    {
        if ( RPC_SESSIONSTATE_CLOSED == m_endpoints[ i ]->GetState() )
            continue;
        ++live;
        if ( m_endpoints[ i ]->GetPeerPrincipal().subject == principal.subject )
            ++ofSubject;
    }
    if ( 0 != m_settings.maxSessions && live >= m_settings.maxSessions )
        return false;
    if ( 0 != m_settings.maxSessionsPerSubject && ofSubject >= m_settings.maxSessionsPerSubject )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::OnPendingFrame( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CIRpcTransportPtr transport;
    if ( GUCEF_NULL == eventData || !TakePending( notifier, transport ) )
        return;
    m_pendingObserver.UnsubscribeFrom( *notifier );
    HandleFirstFrame( transport, *static_cast< CRpcFrameEventData* >( eventData ) );

    // refused, or handed to an endpoint: either way the host may be the last holder, inside the transport's own event
    ReleaseOnItsThread( transport );
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::HandleFirstFrame( const CIRpcTransportPtr& transportRef, const CRpcFrameEventData& eventFrame )
{GUCEF_TRACE;

    CIRpcTransportPtr transport( transportRef );
    const CRpcFrameEventData* frame = &eventFrame;
    SRpcFrameHeader header;
    SRpcHello hello;
    bool isHello = CRpcFrameCodec::ParseHeader( frame->GetFrame(), frame->GetSize(), header ) && GUCEF_RPC_FRAME_HELLO == header.type;
    if ( isHello )
    {
        CORE::CBinaryWireReader reader( frame->GetFrame() + header.bodyOffset, frame->GetSize() - header.bodyOffset );
        isHello = CRpcFrameCodec::ReadHello( reader, hello );
    }
    if ( !isHello )
    {
        // before HELLO_ACK nothing but HELLO is taken
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcHost: the first frame of an accepted transport is not a HELLO, closing it" );
        transport->Close();
        return;
    }

    if ( GUCEF_RPC_HELLO_RESUME == hello.mode )
    {
        CRpcEndpointPtr existing = FindEndpoint( hello.sessionId );
        if ( !existing.IsNULL() )
        {
            existing->AcceptResume( transport, frame->GetFrame(), frame->GetSize() );
            return;
        }
    }
    if ( !MaySessionStart( transport->GetPeerPrincipal() ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcHost: session limit reached, refusing " << transport->GetPeerPrincipal().subject );
        transport->Close();
        return;
    }
    CRpcEndpointPtr endpoint = NewEndpoint();
    {
        MT::CScopeMutex lock( m_lock );
        m_endpoints.push_back( endpoint );
    }
    endpoint->StartAccepted( transport, frame->GetFrame(), frame->GetSize(), GUCEF_RPC_HELLO_RESUME == hello.mode );
    CRpcEndpointEventData data( endpoint );
    NotifyObserversFromThread( EndpointCreatedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::OnEndpointClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRpcEndpointPtr endpoint;
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<m_endpoints.size(); ++i )
        {
            if ( static_cast< CORE::CNotifier* >( m_endpoints[ i ].GetPointerAlways() ) == notifier )
            {
                endpoint = m_endpoints[ i ];
                m_endpoints.erase( m_endpoints.begin() + i );
                break;
            }
        }
    }
    if ( endpoint.IsNULL() )
        return;
    CRpcEndpointEventData data( endpoint );
    NotifyObserversFromThread( EndpointClosedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // accepted transports that send no HELLO in time are closed
    UInt64 now = CRpcDurableRequestTable::NowMs();
    TPendingVector expired;
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<m_pending.size(); )
        {
            if ( 0 != m_settings.helloTimeoutMs && now - m_pending[ i ].sinceMs >= m_settings.helloTimeoutMs )
            {
                expired.push_back( m_pending[ i ] );
                m_pending.erase( m_pending.begin() + i );
            }
            else
                ++i;
        }
    }
    for ( UInt32 i=0; i<expired.size(); ++i )
    {
        m_pendingObserver.UnsubscribeFrom( *expired[ i ].transport.GetPointerAlways() );
        expired[ i ].transport->Close();
    }
    m_durable->Expire();
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::RegisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName,
                           CIRpcServiceImpl* impl, TRpcExecutionMode mode )
{GUCEF_TRACE;

    return m_services->RegisterService( contract, serviceName, impl, mode );
}

/*-------------------------------------------------------------------------*/

CRpcServiceSetPtr
CRpcHost::GetServiceSet( void ) const
{GUCEF_TRACE;

    return m_services;
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::SetAccessPolicy( const CIRpcAccessPolicyPtr& policy )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_accessPolicy = policy;
    for ( UInt32 i=0; i<m_endpoints.size(); ++i )
        m_endpoints[ i ]->SetAccessPolicy( policy );
}

/*-------------------------------------------------------------------------*/

CRpcDurableRequestTablePtr
CRpcHost::GetDurableRequestTable( void ) const
{GUCEF_TRACE;

    return m_durable;
}

/*-------------------------------------------------------------------------*/

bool
CRpcHost::CompleteDurableResult( const CRpcRequestId& id, const CRpcStatus& status, const CRpcMessage* response )
{GUCEF_TRACE;

    CORE::CDynamicBuffer payload;
    if ( GUCEF_NULL != response && status.IsOk() && !response->Encode( payload ) )
        return false;
    return m_durable->Complete( id, status, &payload );
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::GetEndpoints( GUCEF::vector< CRpcEndpointPtr >& endpoints ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    endpoints = m_endpoints;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcHost::GetEndpointCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_endpoints.size();
}

/*-------------------------------------------------------------------------*/

CRpcEndpointPtr
CRpcHost::FindEndpoint( const UInt8* sessionId ) const
{GUCEF_TRACE;

    GUCEF::vector< CRpcEndpointPtr > endpoints;
    GetEndpoints( endpoints );
    UInt8 id[ GUCEF_RPC_SESSIONID_SIZE ];
    for ( UInt32 i=0; i<endpoints.size(); ++i )
    {
        // an endpoint whose session ended (its removal follows in a later pulse) no longer knows it
        TRpcSessionState state = endpoints[ i ]->GetState();
        if ( RPC_SESSIONSTATE_CLOSED == state || RPC_SESSIONSTATE_CLOSING == state )
            continue;
        if ( endpoints[ i ]->GetSessionId( id ) && 0 == memcmp( id, sessionId, GUCEF_RPC_SESSIONID_SIZE ) )
            return endpoints[ i ];
    }
    return CRpcEndpointPtr();
}

/*-------------------------------------------------------------------------*/

void
CRpcHost::GetListenUris( GUCEF::vector< CORE::CUri >& uris ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    uris.clear();
    for ( UInt32 i=0; i<m_listeners.size(); ++i )
        uris.push_back( m_listeners[ i ]->GetListenUri() );
}

/*-------------------------------------------------------------------------*/

CRpcEndpointEventData::CRpcEndpointEventData( const CRpcEndpointPtr& rpcEndpoint )
    : CORE::CICloneable()
    , endpoint( rpcEndpoint )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcEndpointEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcEndpointEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
