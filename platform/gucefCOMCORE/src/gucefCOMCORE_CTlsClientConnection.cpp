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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefCOMCORE_CTlsClientConnection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CTlsClientConnection::HandshakeCompletedEvent = "GUCEF::COMCORE::CTlsClientConnection::HandshakeCompletedEvent";
const CORE::CEvent CTlsClientConnection::TlsErrorEvent = "GUCEF::COMCORE::CTlsClientConnection::TlsErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTlsClientConnection::RegisterEvents( void )
{GUCEF_TRACE;

    HandshakeCompletedEvent.Initialize();
    TlsErrorEvent.Initialize();
    CTCPConnection::RegisterEvents();
    CTCPClientSocket::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTlsClientConnection::CTlsClientConnection( CORE::PulseGeneratorPtr pulseGenerator ,
                                            bool blocking                          )
    : CTCPConnection()
    , m_pulseGenerator( pulseGenerator )
    , m_socket( GUCEF_NEW CTCPClientSocket( pulseGenerator, blocking ) )
    , m_handshakeTimer( GUCEF_NEW CORE::CTimer( pulseGenerator, CTlsSettings::DefaultHandshakeTimeoutInMs ) )
    , m_registry( GUCEF_NULL )
    , m_provider()
    , m_session()
    , m_settings()
    , m_serverName()
    , m_error()
    , m_emptyInfo()
    , m_cipherOut()
    , m_plainIn()
    , m_blockingConnect( false )
    , m_tlsRequested( false )
    , m_upgraded( false )
    , m_established( false )
    , m_failureReported( false )
    , m_localClose( false )
    , m_generation( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CTlsClientConnection::CTlsClientConnection( bool blocking )
    : CTCPConnection()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_socket( GUCEF_NEW CTCPClientSocket( blocking ) )
    , m_handshakeTimer( GUCEF_NEW CORE::CTimer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), CTlsSettings::DefaultHandshakeTimeoutInMs ) )
    , m_registry( GUCEF_NULL )
    , m_provider()
    , m_session()
    , m_settings()
    , m_serverName()
    , m_error()
    , m_emptyInfo()
    , m_cipherOut()
    , m_plainIn()
    , m_blockingConnect( false )
    , m_tlsRequested( false )
    , m_upgraded( false )
    , m_established( false )
    , m_failureReported( false )
    , m_localClose( false )
    , m_generation( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback timerCallback( this, &CTlsClientConnection::OnHandshakeTimer );
    SubscribeTo( m_handshakeTimer              ,
                 CORE::CTimer::TimerUpdateEvent ,
                 timerCallback                 );

    TEventCallback connectingCallback( this, &CTlsClientConnection::OnTcpConnecting );
    SubscribeTo( m_socket                      ,
                 CTCPClientSocket::ConnectingEvent ,
                 connectingCallback            );

    TEventCallback connectedCallback( this, &CTlsClientConnection::OnTcpConnected );
    SubscribeTo( m_socket                      ,
                 CTCPConnection::ConnectedEvent ,
                 connectedCallback             );

    TEventCallback dataReceivedCallback( this, &CTlsClientConnection::OnTcpDataReceived );
    SubscribeTo( m_socket                      ,
                 CTCPConnection::DataRecievedEvent ,
                 dataReceivedCallback          );

    TEventCallback dataSentCallback( this, &CTlsClientConnection::OnTcpDataSent );
    SubscribeTo( m_socket                      ,
                 CTCPConnection::DataSentEvent ,
                 dataSentCallback              );

    TEventCallback disconnectedCallback( this, &CTlsClientConnection::OnTcpDisconnected );
    SubscribeTo( m_socket                      ,
                 CTCPConnection::DisconnectedEvent ,
                 disconnectedCallback          );

    TEventCallback socketErrorCallback( this, &CTlsClientConnection::OnTcpSocketError );
    SubscribeTo( m_socket                      ,
                 CTCPConnection::SocketErrorEvent ,
                 socketErrorCallback           );
}

/*-------------------------------------------------------------------------*/

CTlsClientConnection::~CTlsClientConnection()
{GUCEF_TRACE;

    // Silently: nobody is told about the end of an object that is being destroyed
    UnsubscribeAllFromObserver();
    m_handshakeTimer->SetEnabled( false );
    if ( m_tlsRequested && m_established && m_session && m_socket->IsActive() )
    {
        m_session->InitiateClose();
        SendSessionOutput();
    }
    m_socket->Close();

    // This object can be destroyed by an observer of one of its own events (see ScheduleForDestruction() of the notifiers), which
    // is while the notification of the socket or of the timer that led to that event is still being delivered further up
    // the stack. Those two end after that delivery
    m_handshakeTimer->ScheduleForDestruction();
    m_handshakeTimer = GUCEF_NULL;
    m_socket->ScheduleForDestruction();
    m_socket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::SetProviderRegistry( CTlsProviderRegistry* registry )
{GUCEF_TRACE;

    m_registry = registry;
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::ResetTlsState( void )
{GUCEF_TRACE;

    ++m_generation;
    StopHandshakeTimer();
    m_session.Unlink();
    m_provider.Unlink();
    m_error.Clear();
    m_cipherOut.Clear( true );
    m_plainIn.Clear( true );
    m_tlsRequested = false;
    m_upgraded = false;
    m_established = false;
    m_failureReported = false;
    m_localClose = false;
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::SelectProvider( void )
{GUCEF_TRACE;

    CTlsProviderRegistry* registry = GUCEF_NULL != m_registry ? m_registry : &CComCoreGlobal::Instance()->GetTlsProviderRegistry();

    CORE::CString whyNot;
    m_provider = registry->GetBestAvailable( TLSCAP_NONE, whyNot );
    if ( m_provider.IsNULL() )
    {
        m_error = CTlsError( TLSERR_NO_PROVIDER, whyNot );
        m_error.hint = "Install the OpenSSL runtime of the system (for example the package libssl3), or load a plugin that provides TLS";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TlsClientConnection(" << CORE::PointerToString( this ) << "): No TLS provider is available: " << whyNot );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTlsClientConnection::GetRemoteDescription( void ) const
{GUCEF_TRACE;

    return m_socket->GetRemoteHostName() + ":" + CORE::ToString( m_socket->GetRemoteTCPPort() );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::ConnectTo( const CORE::CString& host    ,
                                 UInt16 port                  ,
                                 bool blocking /* = false */  )
{GUCEF_TRACE;

    return ConnectTo( host, port, GUCEF_NULL, blocking );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::ConnectTo( const CORE::CString& host        ,
                                 UInt16 port                      ,
                                 const CTlsSettings* settings     ,
                                 bool blocking /* = false */      )
{GUCEF_TRACE;

    if ( host.IsNULLOrEmpty() )
        return false;

    m_localClose = true;        // the end of an earlier connection is not a failure of this one
    m_socket->Close();
    ResetTlsState();

    if ( GUCEF_NULL != settings )
    {
        m_settings = *settings;
        m_serverName = m_settings.GetServerName().IsNULLOrEmpty() ? host : m_settings.GetServerName();
        if ( !SelectProvider() )
            return false;

        m_tlsRequested = true;
        if ( !m_settings.IsVerificationEnabled() )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TLS verification is DISABLED for " << host << ":" << port );

        // A TLS record is up to 16 KiB, the socket reads 1 KiB blocks by default
        m_socket->SetMaxRead( TlsReadBlockSize );
    }
    else
    {
        m_socket->SetMaxRead( 0 );
    }

    m_blockingConnect = blocking;
    if ( !m_socket->ConnectTo( host, port, blocking ) )
        return false;

    if ( blocking && m_tlsRequested )
    {
        WaitForHandshake();
        return m_established;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::WaitForHandshake( void )
{GUCEF_TRACE;

    UInt32 generation = m_generation;
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    UInt32 timeoutInMs = m_settings.GetHandshakeTimeoutInMs();

    // The handshake runs on the pulses of the socket, which we cycle here ourselves
    while ( generation == m_generation && m_socket->IsActive() && !m_established && !m_failureReported )
    {
        m_socket->OnPulse( GUCEF_NULL, CORE::CPulseGenerator::PulseEvent, GUCEF_NULL );
        if ( generation != m_generation || !m_socket->IsActive() || m_established || m_failureReported )
            break;

        if ( timeoutInMs > 0 && 0 != ticksPerMs && ( MT::PrecisionTickCount() - start ) / ticksPerMs > timeoutInMs )
        {
            FailTls( CTlsError( TLSERR_HANDSHAKE_TIMEOUT, "the handshake did not finish in " + CORE::ToString( timeoutInMs ) + " ms" ) );
            break;
        }
        MT::PrecisionDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::StartTls( const CTlsSettings& settings    ,
                                const CORE::CString& serverName ,
                                const void* leftover            ,
                                UInt32 leftoverSize             )
{GUCEF_TRACE;

    if ( !m_socket->IsActive() || m_socket->m_isConnecting || m_tlsRequested )
        return false;

    m_error.Clear();
    m_settings = settings;
    m_serverName = serverName.IsNULLOrEmpty() ? m_settings.GetServerName() : serverName;
    if ( !SelectProvider() )
        return false;

    ++m_generation;
    m_tlsRequested = true;
    m_upgraded = true;
    m_established = false;
    m_failureReported = false;
    m_localClose = false;
    if ( !m_settings.IsVerificationEnabled() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TLS verification is DISABLED for " << GetRemoteDescription() );

    m_socket->SetMaxRead( TlsReadBlockSize );
    return BeginHandshake( leftover, leftoverSize );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::BeginHandshake( const void* leftover, UInt32 leftoverSize )
{GUCEF_TRACE;

    CTlsError error;
    m_session = m_provider->CreateClientSession( m_settings, m_serverName, error );
    if ( m_session.IsNULL() )
    {
        if ( !error.IsError() )
            error = CTlsError( TLSERR_INTERNAL, "the TLS provider did not create a session" );
        FailTls( error );
        return false;
    }

    UInt32 generation = m_generation;
    if ( !m_session->Start() )
    {
        FailTls( m_session->GetError() );
        return false;
    }

    UInt32 timeoutInMs = m_settings.GetHandshakeTimeoutInMs();
    if ( timeoutInMs > 0 )
    {
        m_handshakeTimer->SetInterval( timeoutInMs );
        m_handshakeTimer->SetEnabled( true );
    }

    if ( !SendSessionOutput() )
        return false;

    if ( leftoverSize > 0 && GUCEF_NULL != leftover )
    {
        m_session->PushCiphertext( leftover, leftoverSize );
        if ( !ProcessSession() )
            return false;
    }
    return generation == m_generation;
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::StopHandshakeTimer( void )
{GUCEF_TRACE;

    if ( m_handshakeTimer->GetEnabled() )
        m_handshakeTimer->SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::SendSessionOutput( void )
{GUCEF_TRACE;

    m_cipherOut.Clear( true );
    if ( m_session->PullCiphertext( m_cipherOut ) && m_cipherOut.GetDataSize() > 0 )
        return m_socket->Send( m_cipherOut.GetConstBufferPtr(), m_cipherOut.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::FailTls( const CTlsError& error )
{GUCEF_TRACE;

    if ( m_failureReported )
        return false;
    m_failureReported = true;
    m_error = error;
    StopHandshakeTimer();

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TlsClientConnection(" << CORE::PointerToString( this ) << "): TLS with " << GetRemoteDescription()
        << " failed: " << TlsErrorClassToString( error.errorClass ) << ": " << error.message );

    // An alert that tells the peer why goes out before the close
    if ( m_session && m_socket->IsActive() )
        SendSessionOutput();

    // From here on an observer may delete this object: nothing is touched after the notifications
    TTlsErrorEventData errorData( error );
    if ( !NotifyObservers( TlsErrorEvent, &errorData ) )
        return false;

    TSocketErrorEventData socketErrorData( GUCEF_COMCORE_TLS_ERRORCODE_BASE + (Int32) error.errorClass );
    if ( !NotifyObservers( SocketErrorEvent, &socketErrorData ) )
        return false;

    m_localClose = true;    // the disconnect that follows is the consequence of what was just reported
    m_socket->Close();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::ProcessSession( void )
{GUCEF_TRACE;

    UInt32 generation = m_generation;

    // What the peer is waiting for: handshake records, alerts, key updates, tickets
    SendSessionOutput();

    ETlsState state = m_session->GetState();
    if ( TLSSTATE_FAILED == state )
        return FailTls( m_session->GetError() );

    if ( !m_established && ( TLSSTATE_ESTABLISHED == state || TLSSTATE_CLOSING == state || TLSSTATE_CLOSED == state ) )
    {
        m_established = true;
        StopHandshakeTimer();

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TLS established with " << GetRemoteDescription() << ": " << m_session->GetInfo().GetSummary() );

        TTlsSessionInfoEventData infoData( m_session->GetInfo() );
        if ( !NotifyObservers( HandshakeCompletedEvent, &infoData ) )
            return false;
        if ( generation != m_generation || !m_socket->IsActive() )
            return false;

        // A plain connection that was upgraded has been connected already
        if ( !m_upgraded )
        {
            if ( !NotifyObservers( ConnectedEvent ) )
                return false;
            if ( generation != m_generation || !m_socket->IsActive() )
                return false;
        }
    }

    if ( m_established )
    {
        m_plainIn.Clear( true );
        if ( m_session->PullPlaintext( m_plainIn ) && m_plainIn.GetDataSize() > 0 )
        {
            CORE::CDynamicBuffer linkedBuffer;
            linkedBuffer.LinkTo( m_plainIn.GetConstBufferPtr(), m_plainIn.GetDataSize() );
            TDataRecievedEventData receivedData( &linkedBuffer );
            if ( !NotifyObservers( DataRecievedEvent, &receivedData ) )
                return false;
            if ( generation != m_generation || !m_socket->IsActive() )
                return false;
        }

        state = m_session->GetState();
        if ( TLSSTATE_FAILED == state )
            return FailTls( m_session->GetError() );

        if ( TLSSTATE_CLOSED == state )
        {
            // The peer ended the stream properly: we answer in kind and close
            m_localClose = true;
            m_session->InitiateClose();
            SendSessionOutput();
            m_socket->Close();
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::OnSocketData( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    if ( m_session.IsNULL() || m_failureReported )
        return false;

    m_session->PushCiphertext( data.GetConstBufferPtr(), data.GetDataSize() );
    return ProcessSession();
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::OnSocketError( CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Int32 socketErrorCode = 0;
    if ( GUCEF_NULL != eventData )
        socketErrorCode = static_cast< TSocketErrorEventData* >( eventData )->GetData();

    // The socket failed before or during TLS: the details are those of the socket
    if ( !m_failureReported )
    {
        m_failureReported = true;
        StopHandshakeTimer();
        m_error = CTlsError( TLSERR_CONNECT, "socket error " + CORE::ToString( socketErrorCode ) );
        m_error.providerCode = socketErrorCode;

        TTlsErrorEventData errorData( m_error );
        if ( !NotifyObservers( TlsErrorEvent, &errorData ) )
            return false;
    }
    return NotifyObservers( SocketErrorEvent, eventData );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::OnSocketDisconnected( void )
{GUCEF_TRACE;

    StopHandshakeTimer();

    // The peer closed on us while the handshake was going on: that is a failure, there was never a connection to report
    if ( !m_established && !m_failureReported && !m_localClose )
    {
        m_failureReported = true;
        m_error = CTlsError( TLSERR_PEER_CLOSED, "the connection was closed during the handshake" );

        TTlsErrorEventData errorData( m_error );
        if ( !NotifyObservers( TlsErrorEvent, &errorData ) )
            return false;

        TSocketErrorEventData socketErrorData( GUCEF_COMCORE_TLS_ERRORCODE_BASE + (Int32) TLSERR_PEER_CLOSED );
        if ( !NotifyObservers( SocketErrorEvent, &socketErrorData ) )
            return false;
    }
    return NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnHandshakeTimer( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    StopHandshakeTimer();
    if ( !m_session.IsNULL() && !m_established && !m_failureReported )
        FailTls( CTlsError( TLSERR_HANDSHAKE_TIMEOUT, "the handshake did not finish in " + CORE::ToString( m_settings.GetHandshakeTimeoutInMs() ) + " ms" ) );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnTcpConnecting( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnTcpConnected( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // Plain TCP: what the socket says is passed on as it is
    if ( !m_tlsRequested )
    {
        NotifyObservers( eventId, eventData );
        return;
    }

    // The TCP connection is there: the handshake starts and ConnectedEvent follows when it is done
    if ( !m_upgraded )
        BeginHandshake( GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnTcpDataReceived( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_tlsRequested )
    {
        NotifyObservers( eventId, eventData );
        return;
    }

    if ( GUCEF_NULL != eventData )
        OnSocketData( static_cast< CTCPConnection::TDataRecievedEventData* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnTcpDataSent( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // DataSentEvent of the socket is ciphertext when TLS is used: consumers hear about the plaintext they sent from Send()
    if ( !m_tlsRequested )
        NotifyObservers( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnTcpDisconnected( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_tlsRequested )
    {
        NotifyObservers( eventId, eventData );
        return;
    }

    OnSocketDisconnected();
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::OnTcpSocketError( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_tlsRequested )
    {
        NotifyObservers( eventId, eventData );
        return;
    }

    OnSocketError( eventData );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::Send( const void* dataSource ,
                            const UInt32 dataSize  )
{GUCEF_TRACE;

    if ( !m_tlsRequested )
        return m_socket->Send( dataSource, dataSize );

    if ( !m_established || m_session.IsNULL() || m_failureReported || !m_socket->IsActive() )
        return false;
    if ( 0 == dataSize )
        return true;

    if ( TLSSTATE_ESTABLISHED != m_session->PushPlaintext( dataSource, dataSize ) )
    {
        if ( TLSSTATE_FAILED == m_session->GetState() )
            FailTls( m_session->GetError() );
        return false;
    }

    UInt32 generation = m_generation;
    if ( !SendSessionOutput() )
        return false;

    // Told after the data is queued, so that a consumer that sends more from here keeps the order
    CORE::CDynamicBuffer linkedBuffer;
    linkedBuffer.LinkTo( dataSource, dataSize );
    TDataSentEventData sentData( &linkedBuffer );
    if ( !NotifyObservers( DataSentEvent, &sentData ) )
        return false;
    return generation == m_generation;
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::Close( void )
{GUCEF_TRACE;

    m_localClose = true;
    StopHandshakeTimer();
    if ( m_tlsRequested && m_established && !m_session.IsNULL() && m_socket->IsActive() )
    {
        m_session->InitiateClose();
        SendSessionOutput();
    }
    m_socket->Close();
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::IsTlsRequested( void ) const
{GUCEF_TRACE;

    return m_tlsRequested;
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::IsEstablished( void ) const
{GUCEF_TRACE;

    return m_established && m_socket->IsActive();
}

/*-------------------------------------------------------------------------*/

const CTlsSessionInfo&
CTlsClientConnection::GetSessionInfo( void ) const
{GUCEF_TRACE;

    return m_session.IsNULL() ? m_emptyInfo : m_session->GetInfo();
}

/*-------------------------------------------------------------------------*/

const CTlsError&
CTlsClientConnection::GetLastError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CTlsClientConnection::GetRemoteIP( void ) const
{GUCEF_TRACE;

    return m_socket->GetRemoteIP();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsClientConnection::GetRemoteHostName( void ) const
{GUCEF_TRACE;

    return m_socket->GetRemoteHostName();
}

/*-------------------------------------------------------------------------*/

UInt16
CTlsClientConnection::GetRemoteTCPPort( void ) const
{GUCEF_TRACE;

    return m_socket->GetRemoteTCPPort();
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::IsActive( void ) const
{GUCEF_TRACE;

    return m_socket->IsActive();
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsClientConnection::GetBufferedDataToSendInBytes( void ) const
{GUCEF_TRACE;

    return m_socket->GetBufferedDataToSendInBytes();
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::SetUseTcpSendCoalescing( bool coaleseData )
{GUCEF_TRACE;

    return m_socket->SetUseTcpSendCoalescing( coaleseData );
}

/*-------------------------------------------------------------------------*/

bool
CTlsClientConnection::GetUseTcpSendCoalescing( void ) const
{GUCEF_TRACE;

    return m_socket->GetUseTcpSendCoalescing();
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsClientConnection::GetBytesReceived( bool resetCounter )
{GUCEF_TRACE;

    return m_socket->GetBytesReceived( resetCounter );
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsClientConnection::GetBytesTransmitted( bool resetCounter )
{GUCEF_TRACE;

    return m_socket->GetBytesTransmitted( resetCounter );
}

/*-------------------------------------------------------------------------*/

void
CTlsClientConnection::SetMaxUpdatesPerCycle( UInt32 maxUpdates )
{GUCEF_TRACE;

    m_socket->SetMaxUpdatesPerCycle( maxUpdates );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsClientConnection::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CTlsClientConnection";
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
