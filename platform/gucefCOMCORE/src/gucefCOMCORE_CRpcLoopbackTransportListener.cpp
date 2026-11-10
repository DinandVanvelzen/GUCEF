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

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#include "gucefCOMCORE_CRpcLoopbackTransport.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H ? */

#include "gucefCOMCORE_CRpcLoopbackTransportListener.h"

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

namespace {

typedef GUCEF::map< CORE::CString, CRpcLoopbackTransportListener* > TListenerMap;

MT::CMutex g_listenerLock;
TListenerMap g_listeners;

}

/*-------------------------------------------------------------------------*/

CRpcLoopbackTransportListener::CRpcLoopbackTransportListener( void )
    : CIRpcTransportListener()
    , m_notifierLock()
    , m_lock()
    , m_name()
    , m_listening( false )
    , m_pulseGenerator()
    , m_task()
    , m_pulseObserver()
    , m_pendingAccepts()
    , m_roles()
    , m_maxFrameSize( 1024 * 1024 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcLoopbackTransportListener::~CRpcLoopbackTransportListener()
{GUCEF_TRACE;

    StopListening();
    m_pulseObserver.UnsubscribeAllFromObserver();
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    if ( !m_task.IsNULL() )
        m_task->Stop();
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransportListener::SetPrincipalRoles( const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_roles = roles;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransportListener::SetMaxFrameSize( UInt32 maxFrameSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxFrameSize = maxFrameSize;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcLoopbackTransportListener::GetTransportType( void ) const
{GUCEF_TRACE;

    return CRpcLoopbackTransport::TransportType;
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransportListener::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( m_listening )
        return false;
    m_pulseObserver.UnsubscribeAllFromObserver();
    m_pulseGenerator = pulseGenerator;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CRpcLoopbackTransportListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransportListener::Listen( const CORE::CUri& local )
{GUCEF_TRACE;

    if ( local.GetScheme() != CRpcLoopbackTransport::TransportType )
        return false;
    if ( m_pulseGenerator.IsNULL() )
    {
        m_task = CRpcProtocolTask::Start( "gucef.rpc.loopback" );
        if ( m_task.IsNULL() )
            return false;
        m_pulseGenerator = m_task->GetPulseGenerator();
    }
    typedef CORE::CTEventHandlerFunctor< CRpcLoopbackTransportListener > TEventCallback;
    TEventCallback callback( this, &CRpcLoopbackTransportListener::OnPulse );
    m_pulseObserver.SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );

    CORE::CString name = local.GetAuthorityAndPath();
    MT::CScopeMutex globalLock( g_listenerLock );
    if ( g_listeners.find( name ) != g_listeners.end() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcLoopbackTransportListener: loopback://" << name << " is taken" );
        return false;
    }
    g_listeners[ name ] = this;
    m_name = name;
    m_listening = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransportListener::StopListening( void )
{GUCEF_TRACE;

    MT::CScopeMutex globalLock( g_listenerLock );
    if ( !m_listening )
        return;
    TListenerMap::iterator i = g_listeners.find( m_name );
    if ( i != g_listeners.end() && (*i).second == this )
        g_listeners.erase( i );
    m_listening = false;
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransportListener::IsListening( void ) const
{GUCEF_TRACE;

    return m_listening;
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CRpcLoopbackTransportListener::GetListenUri( void ) const
{GUCEF_TRACE;

    return CORE::CUri( "loopback://" + m_name );
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransportListener::ConnectTo( const CORE::CString& name, CRpcLoopbackTransport& connecting )
{GUCEF_TRACE;

    MT::CScopeMutex globalLock( g_listenerLock );
    TListenerMap::iterator i = g_listeners.find( name );
    if ( i == g_listeners.end() )
        return false;
    return (*i).second->Accept( connecting );
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransportListener::Accept( CRpcLoopbackTransport& connecting )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator;
    CORE::CString::StringSet roles;
    UInt32 maxFrameSize = 0;
    CORE::CString name;
    {
        MT::CScopeMutex lock( m_lock );
        pulseGenerator = m_pulseGenerator;
        roles = m_roles;
        maxFrameSize = m_maxFrameSize;
        name = m_name;
    }

    // The accepted side gets the listener's pulse generator but subscribes to it only in OnPulse(), on that thread:
    // this runs on the connecting thread under the table lock of the listeners, and subscribing takes the pulse
    // generator's notification lock, which its thread holds while it pulses (and the listener's lock inside that)
    CORE::CTSharedPtr< CRpcLoopbackTransport, MT::CMutex > shared( GUCEF_NEW CRpcLoopbackTransport() );
    CRpcLoopbackTransport* accepted = shared.GetPointerAlways();
    CIRpcTransportPtr acceptedPtr;
    acceptedPtr = shared;
    accepted->m_pulseGenerator = pulseGenerator;
    accepted->SetPrincipalRoles( roles );
    accepted->SetMaxFrameSize( maxFrameSize );
    accepted->m_deliveryHeld = true;        // until TransportAcceptedEvent announced it, see OnPulse()
    CRpcLoopbackTransport::Join( connecting, *accepted, CORE::CUri( "loopback://" + name ), CORE::CUri( "loopback://" + name + "#client" ) );
    MT::CScopeMutex lock( m_lock );
    m_pendingAccepts.push_back( acceptedPtr );
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->RequestPeriodicPulses( this, 1 );       // accepted in a later pulse, not inside Connect()
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransportListener::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF::vector< CIRpcTransportPtr > accepted;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_pendingAccepts.empty() )
            return;
        accepted.swap( m_pendingAccepts );
        if ( !m_pulseGenerator.IsNULL() )
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    }
    for ( UInt32 i=0; i<accepted.size(); ++i )
    {
        CRpcLoopbackTransport* loopback = static_cast< CRpcLoopbackTransport* >( accepted[ i ].GetPointerAlways() );
        loopback->SubscribeToPulses();

        CRpcTransportAcceptedEventData data( accepted[ i ] );
        NotifyObservers( TransportAcceptedEvent, &data );

        // its observers subscribed in the event: the frames the connecting side sent so far may be delivered now
        loopback->ReleaseDelivery();
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcLoopbackTransportListener::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notifierLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcLoopbackTransportListener::Unlock( void ) const
{GUCEF_TRACE;

    return m_notifierLock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcLoopbackTransportListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcLoopbackTransportListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
