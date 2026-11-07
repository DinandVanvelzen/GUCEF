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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CMailTcpTransport.h"

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

CMailTcpTransport::CMailTcpTransport( void )
    : CIMailTransport()
    , m_connection( GUCEF_NULL )
    , m_reader()
    , m_sendBuffer( 512, true )
    , m_host()
    , m_tlsFailure()
    , m_isConnected( false )
    , m_isSecure( false )
    , m_upgrading( false )
    , m_closing( false )
    , m_generation( 0 )
{GUCEF_TRACE;

    Initialize( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
}

/*-------------------------------------------------------------------------*/

CMailTcpTransport::CMailTcpTransport( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CIMailTransport()
    , m_connection( GUCEF_NULL )
    , m_reader()
    , m_sendBuffer( 512, true )
    , m_host()
    , m_tlsFailure()
    , m_isConnected( false )
    , m_isSecure( false )
    , m_upgrading( false )
    , m_closing( false )
    , m_generation( 0 )
{GUCEF_TRACE;

    Initialize( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator );
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::Initialize( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    m_connection = GUCEF_NEW COMCORE::CTlsClientConnection( pulseGenerator, false );
    RegisterSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::RegisterSocketEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback connectedCallback( this, &CMailTcpTransport::OnSocketConnected );
    SubscribeTo( m_connection, COMCORE::CTCPClientSocket::ConnectedEvent, connectedCallback );

    TEventCallback dataCallback( this, &CMailTcpTransport::OnSocketDataReceived );
    SubscribeTo( m_connection, COMCORE::CTCPClientSocket::DataRecievedEvent, dataCallback );

    TEventCallback disconnectedCallback( this, &CMailTcpTransport::OnSocketDisconnected );
    SubscribeTo( m_connection, COMCORE::CTCPClientSocket::DisconnectedEvent, disconnectedCallback );

    TEventCallback tlsErrorCallback( this, &CMailTcpTransport::OnSocketTlsError );
    SubscribeTo( m_connection, COMCORE::CTlsClientConnection::TlsErrorEvent, tlsErrorCallback );

    TEventCallback errorCallback( this, &CMailTcpTransport::OnSocketError );
    SubscribeTo( m_connection, COMCORE::CTCPClientSocket::SocketErrorEvent, errorCallback );

    TEventCallback handshakeCallback( this, &CMailTcpTransport::OnSocketHandshakeCompleted );
    SubscribeTo( m_connection, COMCORE::CTlsClientConnection::HandshakeCompletedEvent, handshakeCallback );
}

/*-------------------------------------------------------------------------*/

CMailTcpTransport::~CMailTcpTransport()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_connection )
    {
        m_connection->Close();
        m_connection->ScheduleForDestruction();
        m_connection = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::Connect( const CORE::CString& host, UInt16 port, const COMCORE::CTlsSettings* implicitTlsSettings )
{GUCEF_TRACE;

    if ( m_connection->IsActive() )
        m_connection->Close();

    ++m_generation;
    m_reader.Reset();
    m_isConnected = false;
    m_isSecure = GUCEF_NULL != implicitTlsSettings;
    m_upgrading = false;
    m_closing = false;
    m_host = host;
    m_tlsFailure.Clear();

    if ( GUCEF_NULL != implicitTlsSettings )
        return m_connection->ConnectTo( host, port, implicitTlsSettings, false );
    return m_connection->ConnectTo( host, port, false );
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::Disconnect( void )
{GUCEF_TRACE;

    if ( m_connection->IsActive() )
    {
        m_closing = true;
        m_connection->Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::ReportError( EMailErrorClass errorClass, const CORE::CString& description )
{GUCEF_TRACE;

    TTransportErrorEventData eData( errorClass, description );
    NotifyObservers( TransportErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::SendLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( CMimeCodec::HasLineBreakOrNul( line ) )
    {
        ReportError( MAILERR_LOCAL, "A line with a line break or NUL was refused" );
        return false;
    }
    if ( !m_isConnected || !m_connection->IsActive() )
        return false;

    m_sendBuffer.SetDataSize( 0 );
    m_sendBuffer.Append( line.C_String(), line.ByteSize() > 0 ? line.ByteSize() - 1 : 0, true );
    m_sendBuffer.Append( "\r\n", 2, true );
    return m_connection->Send( m_sendBuffer.GetConstBufferPtr(), m_sendBuffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::SendData( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_isConnected || !m_connection->IsActive() )
        return false;
    if ( 0 == size )
        return true;
    return m_connection->Send( data, size );
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::StartTls( const COMCORE::CTlsSettings& settings, const CORE::CString& serverName )
{GUCEF_TRACE;

    if ( !m_isConnected || m_isSecure || !m_connection->IsActive() )
        return false;

    // whatever arrived behind the OK of the server was not sent through TLS
    if ( m_reader.HasBufferedData() )
    {
        ReportError( MAILERR_PROTOCOL, "The server sent data after agreeing to TLS and before the handshake" );
        Disconnect();
        return false;
    }

    m_upgrading = true;
    if ( !m_connection->StartTls( settings, serverName.IsNULLOrEmpty() ? m_host : serverName ) )
    {
        m_upgrading = false;
        ReportError( MAILERR_TLS, "TLS could not be started: " + m_connection->GetLastError().GetDescription( m_host ) );
        Disconnect();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::ExpectBytes( UInt32 count )
{GUCEF_TRACE;

    if ( !m_reader.ExpectBytes( count ) )
    {
        ReportError( MAILERR_PROTOCOL, m_reader.GetOverflowReason() );
        Disconnect();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::ExpectDotTerminatedBytes( UInt32 maxBytes )
{GUCEF_TRACE;

    m_reader.ExpectDotTerminatedBytes( maxBytes );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::SetLimits( UInt32 maxLineLength, UInt32 maxBytes )
{GUCEF_TRACE;

    m_reader.SetLimits( maxLineLength, maxBytes );
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::HasBufferedData( void ) const
{GUCEF_TRACE;

    return m_reader.HasBufferedData();
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::IsConnected( void ) const
{GUCEF_TRACE;

    return m_isConnected && m_connection->IsActive();
}

/*-------------------------------------------------------------------------*/

bool
CMailTcpTransport::IsSecure( void ) const
{GUCEF_TRACE;

    return m_isSecure && IsConnected();
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::Drain( void )
{GUCEF_TRACE;

    const UInt32 generation = m_generation;
    CORE::CString line;
    CORE::CDynamicBuffer bytes;
    for ( ;; )
    {
        CMailStreamReader::EItem item = m_reader.Next( line, bytes );
        if ( CMailStreamReader::ITEM_NONE == item )
            return;

        if ( CMailStreamReader::ITEM_OVERFLOW == item )
        {
            const CORE::CString reason = m_reader.GetOverflowReason();
            ReportError( MAILERR_PROTOCOL, reason );
            Disconnect();
            return;
        }

        if ( CMailStreamReader::ITEM_LINE == item )
        {
            TLineReceivedEventData eData( line );
            if ( !NotifyObservers( LineReceivedEvent, &eData ) )
                return;
        }
        else
        {
            TBytesReceivedEventData eData( &bytes );
            if ( !NotifyObservers( BytesReceivedEvent, &eData ) )
                return;
        }

        // an observer may have closed the connection or started another one
        if ( generation != m_generation || !m_isConnected )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::OnSocketConnected( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_isConnected )
    {
        m_isConnected = true;
        NotifyObservers( ConnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::OnSocketHandshakeCompleted( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_upgrading )
    {
        m_upgrading = false;
        m_isSecure = true;
        NotifyObservers( TlsEstablishedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::OnSocketDataReceived( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    if ( !m_isConnected )
    {
        m_isConnected = true;
        if ( !NotifyObservers( ConnectedEvent ) )
            return;
    }

    const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
    if ( !m_reader.Feed( data.GetConstBufferPtr(), data.GetDataSize() ) )
        return;
    Drain();
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::OnSocketDisconnected( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ++m_generation;
    m_isConnected = false;
    m_isSecure = false;
    m_upgrading = false;
    m_reader.Reset();
    NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::OnSocketTlsError( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the socket error that follows is the report of this one
    COMCORE::TTlsErrorEventData* errorData = static_cast< COMCORE::TTlsErrorEventData* >( eventData );
    if ( GUCEF_NULL != errorData )
        m_tlsFailure = errorData->GetData().GetDescription( m_host );
}

/*-------------------------------------------------------------------------*/

void
CMailTcpTransport::OnSocketError( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const Int32 errorCode = GUCEF_NULL != eventData ? static_cast< COMCORE::CTCPConnection::TSocketErrorEventData* >( eventData )->GetData() : 0;
    if ( !m_tlsFailure.IsNULLOrEmpty() )
    {
        const CORE::CString failure = "TLS: " + m_tlsFailure;
        m_tlsFailure.Clear();
        ReportError( MAILERR_TLS, failure );
    }
    else
        ReportError( MAILERR_CONNECTION, "Socket error " + CORE::Int32ToString( errorCode ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMailTcpTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMailTcpTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
