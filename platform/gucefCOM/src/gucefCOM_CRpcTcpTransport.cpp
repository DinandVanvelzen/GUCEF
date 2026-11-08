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

#include <string.h>

#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#include "gucefCORE_CBinaryWireReader.h"
#define GUCEF_CORE_CBINARYWIREREADER_H
#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#include "gucefCORE_CBinaryWireWriter.h"
#define GUCEF_CORE_CBINARYWIREWRITER_H
#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */

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

#ifndef GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H
#include "gucefCOMCORE_CRpcFrameEventData.h"
#define GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H
#endif /* GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#include "gucefCOM_CRpcTcpTransport.h"

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

const CORE::CString CRpcTcpTransport::TransportType = "tcp";
const UInt8 CRpcTcpTransport::Preface[ 9 ] = { 'G', 'U', 'C', 'E', 'F', 'R', 'P', 'C', 1 };

/*-------------------------------------------------------------------------*/

CRpcTcpTransport::CRpcTcpTransport( void )
    : COMCORE::CIRpcTransport()
    , m_lock()
    , m_outbound()
    , m_sending()
    , m_flushing( false )
    , m_closeRequested( false )
    , m_client( GUCEF_NULL )
    , m_serverConnection( GUCEF_NULL )
    , m_pulseGenerator()
    , m_task()
    , m_observer()
    , m_inbound()
    , m_prefaceReceived( false )
    , m_framesReceived( 0 )
    , m_framesSent( 0 )
    , m_connected( false )
    , m_disconnectRaised( false )
    , m_pulsesRequested( false )
    , m_onOwnTask( false )
    , m_properties()
    , m_principal( TransportType, CORE::CString::Empty, false )
    , m_remoteUri()
    , m_maxFrameSize( DefaultMaxFrameSize )
    , m_maxPreHelloFrameSize( DefaultMaxPreHelloFrameSize )
    , m_allowNonLoopback( false )
    , m_useTls( false )
    , m_tlsSettings()
    , m_threadPoolName( "gucef.rpc.tcp" )
    , m_notifierLock()
{GUCEF_TRACE;

    m_properties.isReliable = true;
    m_properties.isOrdered = true;
    m_properties.isSecure = false;
    m_properties.hasFrameProtectionClasses = false;
    m_properties.maxFrameSize = m_maxFrameSize;
    m_properties.latencyClass = COMCORE::CRpcTransportProperties::LATENCY_LAN;
    m_properties.profileName = TransportType;
}

/*-------------------------------------------------------------------------*/

CRpcTcpTransport::~CRpcTcpTransport()
{GUCEF_TRACE;

    m_observer.UnsubscribeAllFromObserver();
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    if ( GUCEF_NULL != m_client )
    {
        m_client->Close();
        GUCEF_DELETE m_client;
        m_client = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_serverConnection )
    {
        m_serverConnection->Close();
        m_serverConnection = GUCEF_NULL;
    }
    if ( !m_task.IsNULL() )
        m_task->Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTcpTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CRpcTcpTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTcpTransport::GetTransportType( void ) const
{GUCEF_TRACE;

    return TransportType;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::SetMaxFrameSize( UInt32 maxFrameSize )
{GUCEF_TRACE;

    m_maxFrameSize = maxFrameSize;
    m_properties.maxFrameSize = maxFrameSize;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::SetMaxPreHelloFrameSize( UInt32 maxFrameSize )
{GUCEF_TRACE;

    m_maxPreHelloFrameSize = maxFrameSize;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::SetAllowNonLoopback( bool allowNonLoopback )
{GUCEF_TRACE;

    m_allowNonLoopback = allowNonLoopback;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::SetTlsSettings( const COMCORE::CTlsSettings& settings )
{GUCEF_TRACE;

    m_tlsSettings = settings;
    m_useTls = true;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::ClearTlsSettings( void )
{GUCEF_TRACE;

    m_tlsSettings = COMCORE::CTlsSettings();
    m_useTls = false;
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::IsTls( void ) const
{GUCEF_TRACE;

    return m_properties.isSecure || ( GUCEF_NULL == m_serverConnection && m_useTls );
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::IsLoopbackHost( const CORE::CString& host )
{GUCEF_TRACE;

    if ( "localhost" == host.Lowercase() )
        return true;
    CORE::CString::StringVector parts = host.ParseElements( '.', false );
    if ( 4 != parts.size() )
        return false;
    for ( UInt32 i=0; i<parts.size(); ++i )
    {
        if ( parts[ i ].IsNULLOrEmpty() || parts[ i ].Length() > 3 )
            return false;
        for ( UInt32 n=0; n<parts[ i ].Length(); ++n )
            if ( parts[ i ].C_String()[ n ] < '0' || parts[ i ].C_String()[ n ] > '9' )
                return false;
        if ( CORE::StringToInt32( parts[ i ] ) > 255 )
            return false;
    }
    return 127 == CORE::StringToInt32( parts[ 0 ] );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::WriteFramed( CORE::CDynamicBuffer& target, const void* frame, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireWriter writer( target, target.GetDataSize() );
    writer.WriteVarUInt32( size );
    writer.WriteBytes( frame, size );
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    // only before the socket exists: it lives on this thread from then on
    if ( GUCEF_NULL != m_client || GUCEF_NULL != m_serverConnection || pulseGenerator.IsNULL() )
        return false;
    if ( !m_pulseGenerator.IsNULL() )
        m_observer.UnsubscribeFrom( *m_pulseGenerator.GetPointerAlways() );
    m_pulseGenerator = pulseGenerator;
    SubscribeToPulses();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CRpcTcpTransport::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::SubscribeToPulses( void )
{GUCEF_TRACE;

    if ( m_pulseGenerator.IsNULL() )
        return;
    typedef CORE::CTEventHandlerFunctor< CRpcTcpTransport > TEventCallback;
    TEventCallback pulse( this, &CRpcTcpTransport::OnPulse );
    m_observer.SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::EnsurePulseGenerator( void )
{GUCEF_TRACE;

    if ( !m_pulseGenerator.IsNULL() )
        return true;

    // a transport of its own thread: a dedicated task of the task manager, no raw thread
    m_task = COMCORE::CRpcProtocolTask::Start( m_threadPoolName );
    if ( m_task.IsNULL() )
        return false;
    m_pulseGenerator = m_task->GetPulseGenerator();
    m_onOwnTask = true;
    SubscribeToPulses();
    return !m_pulseGenerator.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::IsProtocolThread( void ) const
{GUCEF_TRACE;

    return !m_pulseGenerator.IsNULL() && m_pulseGenerator->GetPulseDriverThreadId() == MT::GetCurrentTaskID();
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::Connect( const CORE::CUri& remote )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client || GUCEF_NULL != m_serverConnection )
        return false;
    CORE::CString host = remote.GetAuthorityHost();
    UInt16 port = (UInt16) CORE::StringToUInt32( remote.GetAuthorityPort() );
    if ( host.IsNULLOrEmpty() || 0 == port )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransport: \"" << remote.ToUriInStringForm() << "\" has no host and port" );
        return false;
    }
    if ( !IsLoopbackHost( host ) && !m_allowNonLoopback && !m_useTls )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransport: " << host << " is not a loopback address, which needs TLS (SetTlsSettings, useTls) or allowNonLoopback" );
        return false;
    }
    if ( !EnsurePulseGenerator() )
        return false;

    m_remoteUri = remote;
    m_principal = COMCORE::CRpcPrincipal( TransportType, "tcp:" + host + ":" + CORE::UInt32ToString( port ), false );
    m_client = GUCEF_NEW COMCORE::CTlsClientConnection( m_pulseGenerator, false );
    m_properties.isSecure = m_useTls;

    // on a task of its own the socket wakes that task when it becomes ready; a pulse generator that was given is
    // serviced at its periodic pulses, whoever gave it knows how it is driven
    m_client->SetUseReadinessWake( m_onOwnTask );
    typedef CORE::CTEventHandlerFunctor< CRpcTcpTransport > TEventCallback;
    TEventCallback connected( this, &CRpcTcpTransport::OnSocketConnected );
    TEventCallback data( this, &CRpcTcpTransport::OnSocketData );
    TEventCallback disconnected( this, &CRpcTcpTransport::OnSocketDisconnected );
    TEventCallback error( this, &CRpcTcpTransport::OnSocketError );
    m_observer.SubscribeTo( m_client, COMCORE::CTCPConnection::ConnectedEvent, connected );
    m_observer.SubscribeTo( m_client, COMCORE::CTCPConnection::DataRecievedEvent, data );
    m_observer.SubscribeTo( m_client, COMCORE::CTCPConnection::DisconnectedEvent, disconnected );
    m_observer.SubscribeTo( m_client, COMCORE::CTCPConnection::SocketErrorEvent, error );
    {
        MT::CScopeMutex lock( m_lock );
        m_outbound.SetDataSize( 0 );
        m_outbound.Append( Preface, sizeof( Preface ) );
        m_closeRequested = false;
        m_disconnectRaised = false;
        m_pulsesRequested = true;
    }
    m_pulseGenerator->RequestPeriodicPulses( this, 5 );
    if ( !m_client->ConnectTo( host, port, m_useTls ? &m_tlsSettings : GUCEF_NULL, false ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcpTransport: cannot connect to " << host << ":" << port <<
                         ( m_useTls && m_client->GetLastError().IsError() ? CORE::CString( ": " ) + m_client->GetLastError().GetDescription( host ) : CORE::CString::Empty ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::Adopt( COMCORE::CTCPServerConnection* connection, const CORE::PulseGeneratorPtr& pulseGenerator, const CORE::CUri& remote, bool onOwnTask )
{GUCEF_TRACE;

    m_serverConnection = connection;
    m_properties.isSecure = connection->IsTls();
    m_pulseGenerator = pulseGenerator;
    m_onOwnTask = onOwnTask;
    m_remoteUri = remote;
    m_principal = COMCORE::CRpcPrincipal( TransportType, "tcp:" + remote.GetAuthorityHost() + ":" + remote.GetAuthorityPort(), false );
    SubscribeToPulses();
    typedef CORE::CTEventHandlerFunctor< CRpcTcpTransport > TEventCallback;
    TEventCallback data( this, &CRpcTcpTransport::OnSocketData );
    TEventCallback disconnected( this, &CRpcTcpTransport::OnSocketDisconnected );
    TEventCallback error( this, &CRpcTcpTransport::OnSocketError );
    m_observer.SubscribeTo( connection, COMCORE::CTCPConnection::DataRecievedEvent, data );
    m_observer.SubscribeTo( connection, COMCORE::CTCPConnection::DisconnectedEvent, disconnected );
    m_observer.SubscribeTo( connection, COMCORE::CTCPConnection::SocketErrorEvent, error );
    {
        MT::CScopeMutex lock( m_lock );
        m_outbound.Append( Preface, sizeof( Preface ) );
        m_connected = true;
        m_pulsesRequested = true;
    }
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestPeriodicPulses( this, 5 );
    FlushOutbound();
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::ConnectionGone( void )
{GUCEF_TRACE;

    // the server socket forgets the connection object: it is not touched again
    if ( GUCEF_NULL != m_serverConnection )
        m_observer.UnsubscribeFrom( *m_serverConnection );
    m_serverConnection = GUCEF_NULL;
    Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the tcp connection closed" ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::Close( void )
{GUCEF_TRACE;

    // handed to the protocol thread: the close and DisconnectedEvent happen at its next pulse
    MT::CScopeMutex lock( m_lock );
    m_closeRequested = true;
    if ( !m_pulsesRequested && !m_pulseGenerator.IsNULL() )
    {
        m_pulsesRequested = true;
        m_pulseGenerator->RequestPeriodicPulses( this, 5 );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_connected && !m_closeRequested;
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::SendFrame( const void* frame, UInt32 size, COMCORE::TRpcProtectionClass protection )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_connected || m_closeRequested || m_disconnectRaised )
        return false;
    UInt32 limit = 0 == m_framesSent ? ( m_maxPreHelloFrameSize > m_maxFrameSize ? m_maxPreHelloFrameSize : m_maxFrameSize ) : m_maxFrameSize;
    if ( size > limit )
        return false;
    bool firstPending = 0 == m_outbound.GetDataSize();
    WriteFramed( m_outbound, frame, size );
    ++m_framesSent;

    // the protocol task writes it at its next pulse: that pulse comes at once, not at the next periodic one. Only the
    // pulse generator's data lock is taken, a leaf. Only on a task of its own: its driver, a task delegator, takes
    // the request without pulsing inside it (a driver that does not loop, such as the busy wait driver outside its
    // loop, would pulse right here)
    if ( firstPending && m_onOwnTask && !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestImmediatePulseThroughDriver();
    return true;
}

/*-------------------------------------------------------------------------*/

const COMCORE::CRpcTransportProperties&
CRpcTcpTransport::GetProperties( void ) const
{GUCEF_TRACE;

    return m_properties;
}

/*-------------------------------------------------------------------------*/

const COMCORE::CRpcPrincipal&
CRpcTcpTransport::GetPeerPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CRpcTcpTransport::GetRemoteUri( void ) const
{GUCEF_TRACE;

    return m_remoteUri;
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::WriteToSocket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return true;
    if ( GUCEF_NULL != m_client )
        return m_client->Send( data, size );
    if ( GUCEF_NULL != m_serverConnection )
        return m_serverConnection->Send( data, size );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::FlushOutbound( void )
{GUCEF_TRACE;

    // protocol thread only; a flush the socket write leads to again leaves the bytes for the next one
    if ( m_flushing )
        return;
    {
        // the queued bytes change places with the buffer written last time: no copy, and once both buffers are large
        // enough no allocation (design 15)
        MT::CScopeMutex lock( m_lock );
        if ( !m_connected || 0 == m_outbound.GetDataSize() )
            return;
        m_sending.SetDataSize( 0 );
        m_sending.Swap( m_outbound );
    }
    m_flushing = true;
    bool written = WriteToSocket( m_sending.GetConstBufferPtr(), m_sending.GetDataSize() );
    m_sending.SetDataSize( 0 );
    m_flushing = false;
    if ( !written )
        Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the tcp connection refused the data" ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    bool close = false;
    {
        MT::CScopeMutex lock( m_lock );
        close = m_closeRequested && !m_disconnectRaised;
    }
    FlushOutbound();
    if ( close )
        Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "closed" ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::OnSocketConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_closeRequested || m_disconnectRaised )
            return;
        m_connected = true;
    }
    FlushOutbound();
    NotifyObservers( ConnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::OnSocketData( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const CORE::CDynamicBuffer& received = static_cast< const COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
    ProcessIncoming( static_cast< const UInt8* >( received.GetConstBufferPtr() ), received.GetDataSize() );
    FlushOutbound();
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::ProcessIncoming( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    if ( m_disconnectRaised )
        return;
    m_inbound.Append( data, size );

    UInt32 consumed = 0;
    const UInt8* bytes = static_cast< const UInt8* >( m_inbound.GetConstBufferPtr() );
    UInt32 available = m_inbound.GetDataSize();
    if ( !m_prefaceReceived )
    {
        // what does not start with the preface is not this protocol: nothing else of it is read
        UInt32 compare = available < sizeof( Preface ) ? available : (UInt32) sizeof( Preface );
        if ( 0 != memcmp( bytes, Preface, compare ) )
        {
            Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the peer does not speak the RPC framing" ) );
            return;
        }
        if ( available < sizeof( Preface ) )
            return;
        m_prefaceReceived = true;
        consumed = sizeof( Preface );
    }

    while ( !m_disconnectRaised && consumed < available )
    {
        CORE::CBinaryWireReader reader( bytes + consumed, available - consumed );
        UInt32 length = 0;
        if ( !reader.ReadVarUInt32( length ) )
        {
            // an incomplete length waits for more bytes; more than 5 bytes of it is not a length
            if ( available - consumed >= 5 )
                Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "a malformed frame length" ) );
            break;
        }
        UInt32 limit = 0 == m_framesReceived ? ( m_maxPreHelloFrameSize > m_maxFrameSize ? m_maxPreHelloFrameSize : m_maxFrameSize ) : m_maxFrameSize;
        if ( 0 == length || length > limit )
        {
            Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, "a frame of " + CORE::UInt32ToString( length ) + " bytes is beyond the limit of " + CORE::UInt32ToString( limit ) ) );
            break;
        }
        if ( reader.GetRemaining() < length )
            break;
        UInt32 frameOffset = consumed + reader.GetPosition();
        ++m_framesReceived;
        consumed = frameOffset + length;
        COMCORE::CRpcFrameEventData frame( bytes + frameOffset, length, COMCORE::RPC_PROTECTION_CONFIDENTIAL );
        NotifyObservers( FrameReceivedEvent, &frame );
    }

    if ( m_disconnectRaised )
    {
        m_inbound.SetDataSize( 0 );
        return;
    }
    if ( consumed > 0 )
    {
        UInt32 rest = m_inbound.GetDataSize() - consumed;
        if ( rest > 0 )
            memmove( m_inbound.GetBufferPtr(), static_cast< const UInt8* >( m_inbound.GetConstBufferPtr() ) + consumed, rest );
        m_inbound.SetDataSize( rest );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::OnSocketDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == m_serverConnection )
    {
        m_observer.UnsubscribeFrom( *m_serverConnection );
        m_serverConnection = GUCEF_NULL;
    }
    Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the tcp connection closed" ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Int32 code = GUCEF_NULL != eventData ? static_cast< const COMCORE::CTCPConnection::TSocketErrorEventData* >( eventData )->GetData() : 0;

    // a failure of TLS: the certificate, the handshake. Its description says what to look at
    CORE::CString what = "tcp socket error " + CORE::Int32ToString( code );
    if ( GUCEF_NULL != m_client && code >= GUCEF_COMCORE_TLS_ERRORCODE_BASE && m_client->GetLastError().IsError() )
        what = "tls: " + m_client->GetLastError().GetDescription( m_remoteUri.GetAuthorityHost() );

    CORE::TCloneableString text( what );
    NotifyObservers( TransportErrorEvent, &text );
    Disconnect( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, what ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcTcpTransport::Disconnect( const COMCORE::CRpcStatus& why )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_disconnectRaised )
            return;
        m_disconnectRaised = true;
        m_connected = false;
        m_outbound.SetDataSize( 0 );
        if ( m_pulsesRequested && !m_pulseGenerator.IsNULL() )
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
        m_pulsesRequested = false;
    }
    if ( GUCEF_NULL != m_client )
    {
        m_observer.UnsubscribeFrom( *m_client );
        m_client->Close();
    }
    if ( GUCEF_NULL != m_serverConnection )
    {
        COMCORE::CTCPServerConnection* connection = m_serverConnection;
        m_observer.UnsubscribeFrom( *connection );
        m_serverConnection = GUCEF_NULL;
        connection->Close();
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcTcpTransport: disconnected from " << m_principal.subject << ": " << why.ToString() );
    COMCORE::CRpcStatus data( why );
    NotifyObservers( DisconnectedEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "maxFrameSize", m_maxFrameSize );
    config.SetAttribute( "maxPreHelloFrameSize", m_maxPreHelloFrameSize );
    config.SetAttribute( "allowNonLoopback", m_allowNonLoopback );
    config.SetAttribute( "threadPoolName", m_threadPoolName );
    config.SetAttribute( "useTls", m_useTls );
    if ( m_useTls )
        m_tlsSettings.SaveConfig( config );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcTcpTransport::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    SetMaxFrameSize( config.GetAttributeValueOrChildValueByName( "maxFrameSize" ).AsUInt32( m_maxFrameSize ) );
    m_maxPreHelloFrameSize = config.GetAttributeValueOrChildValueByName( "maxPreHelloFrameSize" ).AsUInt32( m_maxPreHelloFrameSize );
    m_allowNonLoopback = config.GetAttributeValueOrChildValueByName( "allowNonLoopback" ).AsBool( m_allowNonLoopback );
    m_threadPoolName = config.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_threadPoolName );
    m_useTls = config.GetAttributeValueOrChildValueByName( "useTls" ).AsBool( m_useTls );
    if ( m_useTls )
        m_tlsSettings.LoadConfig( config );
    return true;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcTcpTransport::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notifierLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcTcpTransport::Unlock( void ) const
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
