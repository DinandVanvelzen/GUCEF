/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOM_CIrcTcpTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CIrcTcpTransport::MaxLineSizeInBytes = 8191 + 512;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIrcTcpTransport::CIrcTcpTransport( void )
    : CIIrcTransport()
    , m_socket( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false )
    , m_server()
    , m_tlsFailure()
    , m_receiveBuffer( 1024, true )
    , m_sendBuffer( 512, true )
    , m_isConnected( false )
{GUCEF_TRACE;

    RegisterSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

CIrcTcpTransport::CIrcTcpTransport( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CIIrcTransport()
    , m_socket( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator, false )
    , m_server()
    , m_tlsFailure()
    , m_receiveBuffer( 1024, true )
    , m_sendBuffer( 512, true )
    , m_isConnected( false )
{GUCEF_TRACE;

    RegisterSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CIrcTcpTransport::RegisterSocketEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback connectedCallback( this, &CIrcTcpTransport::OnSocketConnected );
    SubscribeTo( &m_socket                                  ,
                 COMCORE::CTCPClientSocket::ConnectedEvent  ,
                 connectedCallback                          );

    TEventCallback dataCallback( this, &CIrcTcpTransport::OnSocketDataReceived );
    SubscribeTo( &m_socket                                  ,
                 COMCORE::CTCPClientSocket::DataRecievedEvent ,
                 dataCallback                               );

    TEventCallback disconnectedCallback( this, &CIrcTcpTransport::OnSocketDisconnected );
    SubscribeTo( &m_socket                                  ,
                 COMCORE::CTCPClientSocket::DisconnectedEvent ,
                 disconnectedCallback                       );

    TEventCallback tlsErrorCallback( this, &CIrcTcpTransport::OnSocketTlsError );
    SubscribeTo( &m_socket                                  ,
                 COMCORE::CTlsClientConnection::TlsErrorEvent ,
                 tlsErrorCallback                           );

    TEventCallback errorCallback( this, &CIrcTcpTransport::OnSocketError );
    SubscribeTo( &m_socket                                  ,
                 COMCORE::CTCPClientSocket::SocketErrorEvent ,
                 errorCallback                              );
}

/*-------------------------------------------------------------------------*/

CIrcTcpTransport::~CIrcTcpTransport()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

bool
CIrcTcpTransport::Connect( const CIrcClientSettings& settings )
{GUCEF_TRACE;

    if ( m_socket.IsActive() )
        m_socket.Close();

    m_isConnected = false;
    m_receiveBuffer.SetDataSize( 0 );

    m_server = settings.server;
    m_tlsFailure.Clear();

    // Over TLS the server is connected to by name, the name the certificate has to carry
    if ( settings.useTls )
        return m_socket.ConnectTo( settings.server, settings.port, &settings.tls, false );
    return m_socket.ConnectTo( settings.server, settings.port, false );
}

/*-------------------------------------------------------------------------*/

void
CIrcTcpTransport::Disconnect( void )
{GUCEF_TRACE;

    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

bool
CIrcTcpTransport::SendLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( !m_socket.IsActive() )
        return false;

    UInt32 size = line.ByteSize() > 0 ? line.ByteSize() - 1 : 0;
    m_sendBuffer.SetDataSize( 0 );
    m_sendBuffer.Append( line.C_String(), size, true );
    m_sendBuffer.Append( "\r\n", 2, true );
    return m_socket.Send( m_sendBuffer.GetConstBufferPtr(), m_sendBuffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CIrcTcpTransport::IsConnected( void ) const
{GUCEF_TRACE;

    return m_isConnected && m_socket.IsActive();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcTcpTransport::GetTransportName( void ) const
{GUCEF_TRACE;

    return TcpTransportName;
}

/*-------------------------------------------------------------------------*/

bool
CIrcTcpTransport::OnData( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    UInt32 scanStart = m_receiveBuffer.GetDataSize();
    m_receiveBuffer.Append( data.GetConstBufferPtr(), data.GetDataSize(), true );

    const char* buffer = static_cast< const char* >( m_receiveBuffer.GetConstBufferPtr() );
    UInt32 size = m_receiveBuffer.GetDataSize();
    UInt32 lineStart = 0;
    for ( UInt32 i=scanStart; i<size; ++i )
    {
        if ( '\n' != buffer[ i ] )
            continue;

        UInt32 lineEnd = i;
        if ( lineEnd > lineStart && '\r' == buffer[ lineEnd-1 ] )
            --lineEnd;
        if ( lineEnd > lineStart )
        {
            TLineReceivedEventData eData( CORE::CString( buffer + lineStart, lineEnd - lineStart ) );
            if ( !NotifyObservers( LineReceivedEvent, &eData ) )
                return false;

            // An observer may have disconnected us
            if ( !m_socket.IsActive() )
                return true;
            buffer = static_cast< const char* >( m_receiveBuffer.GetConstBufferPtr() );
        }
        lineStart = i + 1;
    }

    if ( lineStart > 0 )
        m_receiveBuffer.Downshift( lineStart );

    if ( m_receiveBuffer.GetDataSize() > MaxLineSizeInBytes )
    {
        TTransportErrorEventData eData( "The server sent a line longer than " + CORE::UInt32ToString( MaxLineSizeInBytes ) + " bytes" );
        if ( !NotifyObservers( TransportErrorEvent, &eData ) )
            return false;
        m_socket.Close();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CIrcTcpTransport::OnSocketConnected( CORE::CNotifier* notifier    ,
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
CIrcTcpTransport::OnSocketDataReceived( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    // Data proves the connection is there, whichever the socket reports first
    if ( !m_isConnected )
    {
        m_isConnected = true;
        if ( !NotifyObservers( ConnectedEvent ) ) return;
    }
    OnData( static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CIrcTcpTransport::OnSocketDisconnected( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_isConnected = false;
    m_receiveBuffer.SetDataSize( 0 );
    NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CIrcTcpTransport::OnSocketTlsError( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the socket error that follows is the report of this one
    COMCORE::TTlsErrorEventData* errorData = static_cast< COMCORE::TTlsErrorEventData* >( eventData );
    if ( GUCEF_NULL != errorData && COMCORE::TLSERR_CONNECT != errorData->GetData().errorClass )
        m_tlsFailure = errorData->GetData().GetDescription( m_server );
}

/*-------------------------------------------------------------------------*/

void
CIrcTcpTransport::OnSocketError( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Int32 errorCode = GUCEF_NULL != eventData ? static_cast< COMCORE::CTCPConnection::TSocketErrorEventData* >( eventData )->GetData() : 0;
    TTransportErrorEventData eData( m_tlsFailure.IsNULLOrEmpty() ? "Socket error " + CORE::Int32ToString( errorCode ) : "TLS: " + m_tlsFailure );
    m_tlsFailure.Clear();
    NotifyObservers( TransportErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcTcpTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CIrcTcpTransport";
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
