/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#include "gucefDRN_CDRNTcpChannel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Packets up to this size are sent with their frame header in one write, larger ones in two */
static const UInt32 SingleWriteLimit = 64 * 1024;

/** Packets sent while a read is delivered are written together, at the latest when this much is waiting */
static const UInt32 CoalescedWriteLimit = 64 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The connection of a channel: the client socket it owns, else the accepted connection */
static inline COMCORE::CTCPConnection*
ChannelConnection( COMCORE::CTCPClientSocket* clientSocket, COMCORE::CTCPServerConnection* serverConnection )
{
    return GUCEF_NULL != clientSocket ? static_cast< COMCORE::CTCPConnection* >( clientSocket )
                                      : static_cast< COMCORE::CTCPConnection* >( serverConnection );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNTcpChannel::CDRNTcpChannel( void )
    : CIDRNChannel()
    , m_clientSocket( GUCEF_NULL )
    , m_serverConnection( GUCEF_NULL )
    , m_pulseGenerator()
    , m_capabilities()
    , m_remote()
    , m_framer( DRN_MAX_STREAM_PACKET_SIZE )
    , m_txBuffer()
    , m_rxView()
    , m_rxEventData( &m_rxView )
    , m_coalesced()
    , m_connected( false )
    , m_closing( false )
    , m_receiving( false )
{GUCEF_TRACE;

    m_capabilities.isReliable = true;
    m_capabilities.isDatagram = false;
    m_capabilities.maxPacketSize = 0;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;
}

/*-------------------------------------------------------------------------*/

CDRNTcpChannel::CDRNTcpChannel( COMCORE::CTCPServerConnection* connection, CORE::PulseGeneratorPtr pulseGenerator )
    : CIDRNChannel()
    , m_clientSocket( GUCEF_NULL )
    , m_serverConnection( connection )
    , m_pulseGenerator( pulseGenerator )
    , m_capabilities()
    , m_remote()
    , m_framer( DRN_MAX_STREAM_PACKET_SIZE )
    , m_txBuffer()
    , m_rxView()
    , m_rxEventData( &m_rxView )
    , m_coalesced()
    , m_connected( false )
    , m_closing( false )
    , m_receiving( false )
{GUCEF_TRACE;

    m_capabilities.isReliable = true;
    m_capabilities.isDatagram = false;
    m_capabilities.maxPacketSize = 0;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;

    if ( GUCEF_NULL != m_serverConnection )
    {
        m_connected = m_serverConnection->IsActive();
        m_serverConnection->SetUseTcpSendCoalescing( false );
        m_remote.SetChannelType( CDRNEndpoint::ChannelTypeTcp );
        m_remote.SetAddress( m_serverConnection->GetRemoteIP().AddressAndPortAsString() );
        SubscribeTo( m_serverConnection );
    }
}

/*-------------------------------------------------------------------------*/

CDRNTcpChannel::~CDRNTcpChannel()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    Close();
    GUCEF_DELETE m_clientSocket;
    m_clientSocket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNTcpChannel::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTcpChannel::Connect( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_serverConnection )
        return false;   // an accepted connection cannot connect elsewhere

    UInt16 port = remote.GetPort();
    CORE::CString host = remote.GetHost();
    if ( 0 == port || host.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTcpChannel: Cannot connect to an endpoint without host and port: " << remote.ToString() );
        return false;
    }

    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    if ( GUCEF_NULL == m_clientSocket )
    {
        m_clientSocket = GUCEF_NEW COMCORE::CTCPClientSocket( m_pulseGenerator, false );
        m_clientSocket->SetUseTcpSendCoalescing( false );
        SubscribeTo( m_clientSocket );
    }

    m_remote = remote;
    m_framer.Reset();
    m_connected = false;
    m_closing = false;
    return m_clientSocket->ConnectTo( host, port, false );
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::DetachFromSocket( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_serverConnection )
    {
        // the connection object of the server is re-used for a later client: let go of it
        UnsubscribeFrom( m_serverConnection );
        m_serverConnection = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::Close( void )
{GUCEF_TRACE;

    // what was sent before the close leaves before it
    m_receiving = false;
    WriteCoalesced();

    m_closing = true;
    m_connected = false;
    if ( GUCEF_NULL != m_clientSocket )
    {
        m_clientSocket->Close();
    }
    if ( GUCEF_NULL != m_serverConnection )
    {
        COMCORE::CTCPServerConnection* connection = m_serverConnection;
        DetachFromSocket();
        connection->Close();
    }
    m_framer.Reset();
}

/*-------------------------------------------------------------------------*/

bool
CDRNTcpChannel::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTcpChannel::SendPacket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_connected || size > DRN_MAX_STREAM_PACKET_SIZE )
        return false;

    COMCORE::CTCPConnection* connection = ChannelConnection( m_clientSocket, m_serverConnection );
    if ( GUCEF_NULL == connection )
        return false;

    UInt8 frameHeader[ CDRNStreamFramer::MaxFrameHeaderSize ];
    UInt32 headerSize = CDRNStreamFramer::WriteFrameHeader( size, frameHeader );

    if ( m_receiving )
    {
        // sent in answer to what is being delivered: one write for all of it once the read is delivered, instead of
        // a write (and the connection's notification of it) per packet
        if ( size <= SingleWriteLimit )
        {
            m_coalesced.Append( frameHeader, headerSize );
            m_coalesced.Append( data, size );
            return m_coalesced.GetDataSize() < CoalescedWriteLimit || WriteCoalesced();
        }
        if ( !WriteCoalesced() )
            return false;
    }

    if ( size <= SingleWriteLimit )
    {
        if ( m_txBuffer.GetBufferSize() < headerSize + size )
            m_txBuffer.SetBufferSize( headerSize + size, false );
        m_txBuffer.CopyFrom( 0, headerSize, frameHeader );
        m_txBuffer.CopyFrom( headerSize, size, data );
        m_txBuffer.SetDataSize( headerSize + size );
        return connection->Send( m_txBuffer.GetConstBufferPtr(), headerSize + size );
    }

    // large packets: no copy of the packet
    return connection->Send( frameHeader, headerSize ) && connection->Send( data, size );
}


/*-------------------------------------------------------------------------*/

bool
CDRNTcpChannel::WriteCoalesced( void )
{GUCEF_TRACE;

    UInt32 size = m_coalesced.GetDataSize();
    if ( 0 == size )
        return true;
    COMCORE::CTCPConnection* connection = ChannelConnection( m_clientSocket, m_serverConnection );
    bool sent = GUCEF_NULL != connection && connection->Send( m_coalesced.GetConstBufferPtr(), size );
    m_coalesced.SetDataSize( 0 );
    return sent;
}

/*-------------------------------------------------------------------------*/

const CDRNChannelCapabilities&
CDRNTcpChannel::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNTcpChannel::GetRemoteEndpoint( void ) const
{GUCEF_TRACE;

    return m_remote;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNTcpChannel::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeTcp;
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::SetMaxReceivePacketSize( UInt32 maxPacketSize )
{GUCEF_TRACE;

    m_framer.SetMaxPacketSize( maxPacketSize < DRN_MAX_STREAM_PACKET_SIZE ? maxPacketSize : DRN_MAX_STREAM_PACKET_SIZE );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNTcpChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNTcpChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::OnData( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    m_framer.BeginChunk( data.GetConstBufferPtr(), data.GetDataSize() );
    m_receiving = true;
    while ( true )
    {
        const UInt8* packet = GUCEF_NULL;
        UInt32 packetSize = 0;
        CDRNStreamFramer::TFrameResult result = m_framer.NextPacket( packet, packetSize );
        if ( CDRNStreamFramer::FRAME_PACKET == result )
        {
            m_rxView.LinkTo( packet, packetSize );
            if ( !NotifyObservers( PacketReceivedEvent, &m_rxEventData ) )
                return;     // the channel is gone, its destruction closed it
            if ( m_closing )
                break;
        }
        else
        if ( CDRNStreamFramer::FRAME_ERROR == result )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTcpChannel: Invalid framing from " << m_remote.ToString() << ", closing the connection" );
            m_framer.Reset();
            CDRNChannelErrorEventData errorData( "invalid framing" );
            m_receiving = false;
            WriteCoalesced();
            if ( !NotifyObservers( ChannelErrorEvent, &errorData ) )
                return;
            Close();
            NotifyObservers( ChannelDisconnectedEvent );
            return;
        }
        else
        {
            break;
        }
    }
    m_framer.EndChunk();
    m_receiving = false;
    WriteCoalesced();
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::OnConnectionLost( void )
{GUCEF_TRACE;

    bool wasConnected = m_connected;
    m_connected = false;
    DetachFromSocket();
    m_framer.Reset();
    if ( !m_closing )
    {
        if ( wasConnected )
        {
            NotifyObservers( ChannelDisconnectedEvent );
        }
        else
        {
            CDRNChannelErrorEventData errorData( "connection failed" );
            NotifyObservers( ChannelErrorEvent, &errorData );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannel::OnNotify( CORE::CNotifier* notifier                 ,
                          const CORE::CEvent& eventid               ,
                          CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    bool fromOurSocket = ( GUCEF_NULL != m_clientSocket && notifier == m_clientSocket ) ||
                         ( GUCEF_NULL != m_serverConnection && notifier == m_serverConnection );
    if ( !fromOurSocket )
        return;

    if ( COMCORE::CTCPConnection::DataRecievedEvent == eventid )
    {
        if ( GUCEF_NULL != eventdata && !m_closing )
        {
            const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventdata )->GetData();
            OnData( data );
        }
    }
    else
    if ( COMCORE::CTCPConnection::ConnectedEvent == eventid )
    {
        if ( !m_closing )
        {
            m_connected = true;
            // the socket exists now: no waiting to coalesce small packets
            if ( GUCEF_NULL != m_clientSocket )
                m_clientSocket->SetUseTcpSendCoalescing( false );
            NotifyObservers( ChannelConnectedEvent );
        }
    }
    else
    if ( COMCORE::CTCPConnection::DisconnectedEvent == eventid || CORE::CNotifier::DestructionEvent == eventid )
    {
        OnConnectionLost();
    }
    else
    if ( COMCORE::CTCPConnection::SocketErrorEvent == eventid )
    {
        if ( !m_closing )
        {
            Int32 errorCode = GUCEF_NULL != eventdata ? static_cast< COMCORE::CTCPConnection::TSocketErrorEventData* >( eventdata )->GetData() : 0;
            CDRNChannelErrorEventData errorData( "socket error " + CORE::Int32ToString( errorCode ) );
            if ( !NotifyObservers( ChannelErrorEvent, &errorData ) )
                return;
            if ( !m_connected )
            {
                // a failed connect: there will be no connection
                m_closing = true;
                if ( GUCEF_NULL != m_clientSocket )
                    m_clientSocket->Close();
            }
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
