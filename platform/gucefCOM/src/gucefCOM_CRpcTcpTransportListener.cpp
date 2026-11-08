/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOM_CRpcTcpTransportListener.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcTcpTransportListener::CRpcTcpTransportListener( void )
    : COMCORE::CIRpcTransportListener()
    , m_socket( GUCEF_NULL )
    , m_transports()
    , m_retired()
    , m_pulseGenerator()
    , m_task()
    , m_host()
    , m_listening( false )
    , m_maxConnections( DefaultMaxConnections )
    , m_maxFrameSize( CRpcTcpTransport::DefaultMaxFrameSize )
    , m_maxPreHelloFrameSize( CRpcTcpTransport::DefaultMaxPreHelloFrameSize )
    , m_allowNonLoopback( false )
    , m_threadPoolName( "gucef.rpc.tcp" )
    , m_notifierLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcTcpTransportListener::~CRpcTcpTransportListener()
{GUCEF_TRACE;

    StopListening();
    UnsubscribeAllFromObserver();
    if ( !m_task.IsNULL() )
        m_task->Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTcpTransportListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CRpcTcpTransportListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTcpTransportListener::GetTransportType( void ) const
{GUCEF_TRACE;

    return CRpcTcpTransport::TransportType;
}

/*-------------------------------------------------------------------------*/

void CRpcTcpTransportListener::SetMaxConnections( UInt32 maxConnections ) {GUCEF_TRACE; m_maxConnections = maxConnections; }
void CRpcTcpTransportListener::SetMaxFrameSize( UInt32 maxFrameSize ) {GUCEF_TRACE; m_maxFrameSize = maxFrameSize; }
void CRpcTcpTransportListener::SetAllowNonLoopback( bool allowNonLoopback ) {GUCEF_TRACE; m_allowNonLoopback = allowNonLoopback; }

/*-------------------------------------------------------------------------*/

UInt32
CRpcTcpTransportListener::GetConnectionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_transports.size();
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransportListener::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( m_listening || pulseGenerator.IsNULL() )
        return false;
    m_pulseGenerator = pulseGenerator;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CRpcTcpTransportListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransportListener::IsListening( void ) const
{GUCEF_TRACE;

    return m_listening;
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CRpcTcpTransportListener::GetListenUri( void ) const
{GUCEF_TRACE;

    UInt16 port = GUCEF_NULL != m_socket ? m_socket->GetBoundPort() : 0;
    return CORE::CUri( "tcp://" + m_host + ":" + CORE::UInt32ToString( port ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransportListener::Listen( const CORE::CUri& local )
{GUCEF_TRACE;

    if ( m_listening )
        return false;
    m_host = local.GetAuthorityHost();
    if ( m_host.IsNULLOrEmpty() )
        m_host = "127.0.0.1";
    UInt16 port = (UInt16) CORE::StringToUInt32( local.GetAuthorityPort() );
    if ( !CRpcTcpTransport::IsLoopbackHost( m_host ) )
    {
        if ( !m_allowNonLoopback )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransportListener: " << m_host << " is not a loopback address, which needs allowNonLoopback; there is no TLS for this transport" );
            return false;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransportListener: listening on " << m_host << ", which is not a loopback address, without TLS" );
    }
    if ( m_pulseGenerator.IsNULL() )
    {
        // a listener of its own thread: a dedicated task of the task manager, no raw thread
        m_task = COMCORE::CRpcProtocolTask::Start( m_threadPoolName );
        if ( m_task.IsNULL() )
            return false;
        m_pulseGenerator = m_task->GetPulseGenerator();
    }

    m_socket = GUCEF_NEW COMCORE::CTCPServerSocket( m_pulseGenerator, false, m_maxConnections );

    // on a task of its own the sockets wake that task when they become ready; a pulse generator that was given is
    // serviced at its periodic pulses, whoever gave it knows how it is driven
    m_socket->SetUseReadinessWake( !m_task.IsNULL() );
    if ( !m_socket->SetBindAddress( "localhost" == m_host.Lowercase() ? CORE::CString( "127.0.0.1" ) : m_host ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransportListener: the bind address " << m_host << " is not valid" );
        GUCEF_DELETE m_socket;
        m_socket = GUCEF_NULL;
        return false;
    }
    typedef CORE::CTEventHandlerFunctor< CRpcTcpTransportListener > TEventCallback;
    TEventCallback connected( this, &CRpcTcpTransportListener::OnClientConnected );
    TEventCallback disconnected( this, &CRpcTcpTransportListener::OnClientDisconnected );
    TEventCallback error( this, &CRpcTcpTransportListener::OnSocketError );
    TEventCallback pulse( this, &CRpcTcpTransportListener::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
    SubscribeTo( m_socket, COMCORE::CTCPServerSocket::ClientConnectedEvent, connected );
    SubscribeTo( m_socket, COMCORE::CTCPServerSocket::ClientDisconnectedEvent, disconnected );
    SubscribeTo( m_socket, COMCORE::CTCPServerSocket::ServerSocketErrorEvent, error );
    m_pulseGenerator->RequestPeriodicPulses( this, 5 );
    if ( !m_socket->ListenOnPort( port ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransportListener: cannot listen on " << m_host << ":" << port );
        StopListening();
        return false;
    }
    m_listening = true;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransportListener: listening on " << GetListenUri().ToUriInStringForm() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransportListener::StopListening( void )
{GUCEF_TRACE;

    // a task of its own pulses the socket and the transports of its connections: it ends first, so that nothing
    // touches them on that thread while they go here (a listener on a pulse generator it was given is stopped on the
    // thread of that pulse generator)
    bool ownTask = !m_task.IsNULL();
    if ( ownTask )
    {
        m_task->Stop();
        m_task.Unlink();
    }

    TTransportMap transports;
    transports.swap( m_transports );
    TTransportMap::iterator i = transports.begin();
    for ( ; i != transports.end(); ++i )
        (*i).second->ConnectionGone();
    transports.clear();
    m_retired.clear();
    if ( GUCEF_NULL != m_socket )
    {
        UnsubscribeFrom( m_socket );
        m_socket->Close();
        GUCEF_DELETE m_socket;
        m_socket = GUCEF_NULL;
    }
    if ( !m_pulseGenerator.IsNULL() )
    {
        UnsubscribeFrom( m_pulseGenerator.GetPointerAlways() );
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    }
    if ( ownTask )
        m_pulseGenerator.Unlink();      // that of the task: a next Listen() starts a task again
    m_listening = false;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransportListener::OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const COMCORE::CTCPServerSocket::TConnectionInfo& info = static_cast< const COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData )->GetData();
    COMCORE::CTCPServerConnection* connection = info.connection;

    // a connection object the socket reuses: what it carried before is gone
    TTransportMap::iterator stale = m_transports.find( connection );
    if ( stale != m_transports.end() )
    {
        CRpcTcpTransportPtr old = (*stale).second;
        m_transports.erase( stale );
        m_retired.push_back( old );
        old->ConnectionGone();
    }

    CRpcTcpTransportPtr transport( GUCEF_NEW CRpcTcpTransport() );
    transport->SetMaxFrameSize( m_maxFrameSize );
    transport->SetMaxPreHelloFrameSize( m_maxPreHelloFrameSize );
    transport->Adopt( connection, m_pulseGenerator, CORE::CUri( "tcp://" + info.hostAddress.HostnameAndPortAsString() ), !m_task.IsNULL() );
    m_transports[ connection ] = transport;

    COMCORE::CIRpcTransportPtr accepted;
    accepted = transport;
    COMCORE::CRpcTransportAcceptedEventData data( accepted );
    NotifyObservers( TransportAcceptedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransportListener::OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const COMCORE::CTCPServerSocket::TDisconnectInfo& info = static_cast< const COMCORE::CTCPServerSocket::TClientDisconnectedEventData* >( eventData )->GetData();
    TTransportMap::iterator found = m_transports.find( info.connectionInfo.connection );
    if ( found == m_transports.end() )
        return;
    CRpcTcpTransportPtr transport = (*found).second;
    m_transports.erase( found );
    m_retired.push_back( transport );
    transport->ConnectionGone();
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransportListener::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the connection that closed inside one of its own events is let go here, outside them
    m_retired.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransportListener::OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Int32 code = GUCEF_NULL != eventData ? static_cast< const COMCORE::CTCPServerSocket::TServerSocketErrorEventData* >( eventData )->GetData() : 0;
    CORE::TCloneableString text( "RpcTcpTransportListener: server socket error " + CORE::Int32ToString( code ) );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, text );
    NotifyObservers( ListenerErrorEvent, &text );
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransportListener::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "maxConnections", m_maxConnections );
    config.SetAttribute( "maxFrameSize", m_maxFrameSize );
    config.SetAttribute( "maxPreHelloFrameSize", m_maxPreHelloFrameSize );
    config.SetAttribute( "allowNonLoopback", m_allowNonLoopback );
    config.SetAttribute( "threadPoolName", m_threadPoolName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransportListener::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_maxConnections = config.GetAttributeValueOrChildValueByName( "maxConnections" ).AsUInt32( m_maxConnections );
    m_maxFrameSize = config.GetAttributeValueOrChildValueByName( "maxFrameSize" ).AsUInt32( m_maxFrameSize );
    m_maxPreHelloFrameSize = config.GetAttributeValueOrChildValueByName( "maxPreHelloFrameSize" ).AsUInt32( m_maxPreHelloFrameSize );
    m_allowNonLoopback = config.GetAttributeValueOrChildValueByName( "allowNonLoopback" ).AsBool( m_allowNonLoopback );
    m_threadPoolName = config.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_threadPoolName );
    return true;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcTcpTransportListener::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notifierLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcTcpTransportListener::Unlock( void ) const
{GUCEF_TRACE;

    return m_notifierLock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
