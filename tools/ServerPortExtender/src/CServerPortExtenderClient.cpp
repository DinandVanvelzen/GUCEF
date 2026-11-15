/*
 *  ServerPortExtender: Generic networking tool for connecting clients
 *  to a server that is behind a NAT firewall.
 *
 *  Copyright (C) 2002 - 2009.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#include "CServerPortExtenderClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define SPE_RECONNECT_INTERVAL  5000

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CServerPortExtenderClient::CServerPortExtenderClient( CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::CObserver()                        ,
      m_controlClient( pulseGenerator, false ) ,
      m_rsClientConnections()                  ,
      m_localClientConnections()               ,
      m_remoteToLocalConnectionMap()           ,
      m_localToRemoteConnectionMap()           ,
      m_localServer()                          ,
      m_remoteSPEServerControl()               ,
      m_remoteSPEReversedServer()              ,
      m_controlConnectionInitialized( false )  ,
      m_pulseGenerator( pulseGenerator )       ,
      m_reconnectTimer( pulseGenerator, SPE_RECONNECT_INTERVAL ) ,
      m_disconnectRequested( false )           ,
      m_clientGarbageHeap()
{GUCEF_TRACE;

    m_remoteSPEServerControl.SetPortInHostByteOrder( 10236 );
    m_remoteSPEReversedServer.SetPortInHostByteOrder( 10235 );
    m_localServer.SetPortInHostByteOrder( 10234 );

    // Subscribe to control client events
    TEventCallback callback( this, &CServerPortExtenderClient::OnControlClientConnected );
    SubscribeTo( &m_controlClient                          ,
                 COMCORE::CTCPClientSocket::ConnectedEvent ,
                 callback                                  );
    TEventCallback callback2( this, &CServerPortExtenderClient::OnControlClientDisconnected );
    SubscribeTo( &m_controlClient                             ,
                 COMCORE::CTCPClientSocket::DisconnectedEvent ,
                 callback2                                    );
    TEventCallback callback3( this, &CServerPortExtenderClient::OnControlClientDataSent );
    SubscribeTo( &m_controlClient                         ,
                 COMCORE::CTCPClientSocket::DataSentEvent ,
                 callback3                                );
    TEventCallback callback4( this, &CServerPortExtenderClient::OnControlClientSocketError );
    SubscribeTo( &m_controlClient                            ,
                 COMCORE::CTCPClientSocket::SocketErrorEvent ,
                 callback4                                   );
    TEventCallback callback5( this, &CServerPortExtenderClient::OnControlClientDataRecieved );
    SubscribeTo( &m_controlClient                             ,
                 COMCORE::CTCPClientSocket::DataRecievedEvent ,
                 callback5                                    );

    // Subscribe to Pulse
    TEventCallback callback6( this, &CServerPortExtenderClient::OnPulse );
    SubscribeTo( pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent ,
                 callback6                         );

    // Request to be periodicly updated
    pulseGenerator->RequestPeriodicPulses( this, 10 );

    // Subscribe to timer
    TEventCallback callback7( this, &CServerPortExtenderClient::OnReconnectTimerUpdate );
    SubscribeTo( &m_reconnectTimer              ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback7                      );

    // Subscribe to application
    TEventCallback callback8( this, &CServerPortExtenderClient::OnAppShutdown );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetApplication() ,
                 CORE::CGUCEFApplication::AppShutdownEvent        ,
                 callback8                                        );
}

/*-------------------------------------------------------------------------*/

CServerPortExtenderClient::~CServerPortExtenderClient()
{GUCEF_TRACE;

    Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CServerPortExtenderClient::ConnectToSPEControlSocket( const COMCORE::CHostAddress& host )
{GUCEF_TRACE;

    m_disconnectRequested = false;
    m_controlConnectionInitialized = false;
    m_remoteSPEServerControl = host;
    m_remoteSPEReversedServer.SetHostname( host.GetHostname() );
    return m_controlClient.ConnectTo( host );
}

/*-------------------------------------------------------------------------*/

bool
CServerPortExtenderClient::ConnectToSPEControlSocket( const CORE::CString& hostname ,
                                                      CORE::UInt16 controlPort      )
{GUCEF_TRACE;

    COMCORE::CHostAddress host( hostname, controlPort );
    return ConnectToSPEControlSocket( host );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::SetLocalServer( const COMCORE::CHostAddress& host )
{GUCEF_TRACE;

    m_localServer = host;
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::SetLocalServer( const CORE::CString& hostname ,
                                           CORE::UInt16 port             )
{GUCEF_TRACE;

    COMCORE::CHostAddress host( hostname, port );
    SetLocalServer( host );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::SetRemoteServerSocket( CORE::UInt16 port )
{GUCEF_TRACE;

    m_remoteSPEReversedServer = m_remoteSPEServerControl;
    m_remoteSPEReversedServer.SetPortInHostByteOrder( port );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::RemoveConnectionUsingLocalClient( COMCORE::CTCPClientSocket* localSocket )
{GUCEF_TRACE;

    COMCORE::CTCPClientSocket* speClientSocket = m_localToRemoteConnectionMap[ localSocket->GetSocketID() ];
    m_localToRemoteConnectionMap.erase( localSocket->GetSocketID() );

    if ( GUCEF_NULL != speClientSocket )
    {
        m_remoteToLocalConnectionMap.erase( speClientSocket->GetSocketID() );
        speClientSocket->Close();
        m_rsClientConnections.erase( speClientSocket );
        m_clientGarbageHeap.insert( speClientSocket );
    }

    localSocket->Close();
    m_localClientConnections.erase( localSocket );
    m_clientGarbageHeap.insert( localSocket );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::RemoveConnectionUsingSPEClient( COMCORE::CTCPClientSocket* speClientSocket )
{GUCEF_TRACE;

    COMCORE::CTCPClientSocket* localSocket = m_remoteToLocalConnectionMap[ speClientSocket->GetSocketID() ];
    m_remoteToLocalConnectionMap.erase( speClientSocket->GetSocketID() );

    if ( GUCEF_NULL != localSocket )
    {
        m_localToRemoteConnectionMap.erase( localSocket->GetSocketID() );
        localSocket->Close();
        m_localClientConnections.erase( localSocket );
        m_clientGarbageHeap.insert( localSocket );
    }

    speClientSocket->Close();
    m_rsClientConnections.erase( speClientSocket );
    m_clientGarbageHeap.insert( speClientSocket );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::CleanupGarbage( void )
{GUCEF_TRACE;

    if ( !m_clientGarbageHeap.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Cleaning up discarded client sockets" );
    }

    TSocketSet::iterator i = m_clientGarbageHeap.begin();
    while ( i != m_clientGarbageHeap.end() )
    {
        delete (*i);
        m_clientGarbageHeap.erase( i );

        i = m_clientGarbageHeap.begin();
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::Disconnect( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Disconnecting,.." );

    m_disconnectRequested = true;
    m_controlClient.Close();

    TClientConnectionBufferMap::iterator i = m_localClientConnections.begin();
    while ( i != m_localClientConnections.end() )
    {
        RemoveConnectionUsingLocalClient( (*i).first );
        i = m_localClientConnections.begin();
    }

    m_localClientConnections.clear();
    m_rsClientConnections.clear();
    m_remoteToLocalConnectionMap.clear();
    m_localToRemoteConnectionMap.clear();

    CleanupGarbage();
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnAppShutdown( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventid  ,
                                          CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Disconnecting because the application is shutting down" );

    Disconnect();
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnReconnectTimerUpdate( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventid  ,
                                                   CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // Unless we have been asked to disconnect we will always attempt to keep the
    // control connection up and running
    m_reconnectTimer.SetEnabled( false );
    if ( !m_disconnectRequested )
    {
        if ( !m_controlClient.IsActive() )
		{
			GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Attempting to reconnect control connection,.." );
			m_controlClient.Reconnect();
		}
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnPulse( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventid  ,
                                    CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // transfer data buffered from local clients to the remote SPE server
    CORE::CDynamicBuffer transferBuffer;
    TClientConnectionBufferMap::iterator i = m_localClientConnections.begin();
    while ( i != m_localClientConnections.end() )
    {
        // Check to see if we need to transfer data for this connection to its linked connection
        CORE::CCyclicDynamicBuffer& cBuffer = (*i).second;
        if ( cBuffer.HasBufferedData() )
        {
            // Since we have data to transfer check if the corresponding connection is available for transfer
            COMCORE::CTCPClientSocket* connection = GetRemoteConnectionForLocalConnection( (*i).first->GetSocketID() );
            if ( GUCEF_NULL != connection )
            {
                // transfer the data
                CORE::UInt32 bytesRead;
                do
                {
                    bytesRead = (*i).second.ReadBlockTo( transferBuffer );
                    if ( bytesRead > 0 )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Fowarding buffered client data to remote SPE server" );
                        connection->Send( transferBuffer.GetConstBufferPtr(), transferBuffer.GetDataSize() );
                    }
                }
                while ( bytesRead > 0 );
            }
        }
        ++i;
    }

    // transfer data buffered from the remote SPE server to local clients
    i = m_rsClientConnections.begin();
    while ( i != m_rsClientConnections.end() )
    {
        // Check to see if we need to transfer data for this connection to its linked connection
        CORE::CCyclicDynamicBuffer& cBuffer = (*i).second;
        if ( cBuffer.HasBufferedData() )
        {
            // Since we have data to transfer check if the corresponding connection is available for transfer
            COMCORE::CTCPClientSocket* connection = GetLocalConnectionForRemoteConnection( (*i).first->GetSocketID() );
            if ( GUCEF_NULL != connection )
            {
                // transfer the data
                CORE::UInt32 bytesRead;
                do
                {
                    bytesRead = (*i).second.ReadBlockTo( transferBuffer );
                    if ( bytesRead > 0 )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Fowarding buffered remote SPE server data to local client" );
                        connection->Send( transferBuffer.GetConstBufferPtr(), transferBuffer.GetDataSize() );
                    }
                }
                while ( bytesRead > 0 );
            }
        }
        ++i;
    }

    CleanupGarbage();
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToActualServerConnected( CORE::CNotifier* notifier    ,
                                                            const CORE::CEvent& eventid  ,
                                                            CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: A connection to the actual server has been established" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToActualServerDisconnected( CORE::CNotifier* notifier    ,
                                                               const CORE::CEvent& eventid  ,
                                                               CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: A connection to the actual server has been disconnected" );

    // make sure we dismantle our connection links
    RemoveConnectionUsingLocalClient( static_cast< COMCORE::CTCPClientSocket* >( notifier ) );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToActualServerDataRecieved( CORE::CNotifier* notifier    ,
                                                               const CORE::CEvent& eventid  ,
                                                               CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // received data from the actual server on this client connection.
    // we have to forward it to remote SPE server
    COMCORE::CTCPClientSocket* localClientConnection = static_cast< COMCORE::CTCPClientSocket* >( notifier );
    COMCORE::CTCPClientSocket* remoteClientConnection = GetRemoteConnectionForLocalConnection( localClientConnection->GetSocketID() );

    if ( GUCEF_NULL != remoteClientConnection )
    {
        // get the received data
        COMCORE::CTCPClientSocket::TDataRecievedEventData* receivedDataObj = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
        const CORE::CDynamicBuffer& receivedData = receivedDataObj->GetData();

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Received " + CORE::UInt32ToString( receivedData.GetDataSize() ) + " bytes of data from the actual server, forwarding it" );

        // write the received data into our data buffer for this socket
        CORE::CCyclicDynamicBuffer& buffer = m_rsClientConnections[ remoteClientConnection ];
        buffer.Write( receivedData.GetConstBufferPtr(), receivedData.GetDataSize(), 1 );

        // send all data from the data buffer
        SendAllDataInBuffer( buffer, *remoteClientConnection );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToActualServerDataSent( CORE::CNotifier* notifier    ,
                                                           const CORE::CEvent& eventid  ,
                                                           CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ServerPortExtenderClient: Sending data to the actual server" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToActualServerSocketError( CORE::CNotifier* notifier    ,
                                                              const CORE::CEvent& eventid  ,
                                                              CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    CORE::Int32 socketErrorCode = static_cast< COMCORE::CTCPClientSocket::TSocketErrorEventData* >( eventdata )->GetData();
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ServerPortExtenderClient: Socket error on a connection with the actual server: " + CORE::Int32ToString( socketErrorCode ) );

    // Remove the corresponding remote connection
    COMCORE::CTCPClientSocket* localClientConnection = static_cast< COMCORE::CTCPClientSocket* >( notifier );
    RemoveConnectionUsingLocalClient( localClientConnection );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToRemoteSPEConnected( CORE::CNotifier* notifier    ,
                                                         const CORE::CEvent& eventid  ,
                                                         CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // We just managed to connect to the remote SPE server
    // Now we should connect to the real server to complete the connection

    // initiate a new connection to the local server
    // we create the client socket and add it to our list of connections
    COMCORE::CTCPClientSocket* clientSocket = new COMCORE::CTCPClientSocket( m_pulseGenerator, false );

    // Subscribe to client events
    TEventCallback callback( this, &CServerPortExtenderClient::OnClientToActualServerConnected );
    SubscribeTo( clientSocket                              ,
                 COMCORE::CTCPClientSocket::ConnectedEvent ,
                 callback                                  );
    TEventCallback callback2( this, &CServerPortExtenderClient::OnClientToActualServerDisconnected );
    SubscribeTo( clientSocket                                 ,
                 COMCORE::CTCPClientSocket::DisconnectedEvent ,
                 callback2                                    );
    TEventCallback callback3( this, &CServerPortExtenderClient::OnClientToActualServerDataSent );
    SubscribeTo( clientSocket                             ,
                 COMCORE::CTCPClientSocket::DataSentEvent ,
                 callback3                                );
    TEventCallback callback4( this, &CServerPortExtenderClient::OnClientToActualServerSocketError );
    SubscribeTo( clientSocket                                ,
                 COMCORE::CTCPClientSocket::SocketErrorEvent ,
                 callback4                                   );
    TEventCallback callback5( this, &CServerPortExtenderClient::OnClientToActualServerDataRecieved );
    SubscribeTo( clientSocket                                 ,
                 COMCORE::CTCPClientSocket::DataRecievedEvent ,
                 callback5                                    );

    // add to list of connections and create buffer
    m_localClientConnections[ clientSocket ];

    // map the local to remote client connections
    MapLocalToRemoteConnection( clientSocket, static_cast< COMCORE::CTCPClientSocket* >( notifier ) );

    clientSocket->ConnectTo( m_localServer );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Received request for a new connection to remote SPE server, created client and attempting to connect" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToRemoteSPEDisconnected( CORE::CNotifier* notifier    ,
                                                            const CORE::CEvent& eventid  ,
                                                            CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // make sure we dismantle our connection links
    RemoveConnectionUsingSPEClient( static_cast< COMCORE::CTCPClientSocket* >( notifier ) );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::SendAllDataInBuffer( CORE::CCyclicDynamicBuffer& buffer ,
                                                COMCORE::CTCPClientSocket& socket  )
{GUCEF_TRACE;

    CORE::CDynamicBuffer dataBlock;
    while ( buffer.ReadBlockTo( dataBlock ) > 0 )
    {
        socket.Send( dataBlock.GetConstBufferPtr(), dataBlock.GetDataSize() );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToRemoteSPEDataRecieved( CORE::CNotifier* notifier    ,
                                                            const CORE::CEvent& eventid  ,
                                                            CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Received data from remote SPE server" );

    // received data from the SPE server on this client connection.
    // we have to forward it to our local server
    COMCORE::CTCPClientSocket* remoteConnection = static_cast< COMCORE::CTCPClientSocket* >( notifier );

    // get the received data
    COMCORE::CTCPClientSocket::TDataRecievedEventData* receivedDataObj = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
    const CORE::CDynamicBuffer& receivedData = receivedDataObj->GetData();

    // write the received data into our data buffer for this socket
    CORE::CCyclicDynamicBuffer& buffer = m_rsClientConnections[ remoteConnection ];
    buffer.Write( receivedData.GetConstBufferPtr(), receivedData.GetDataSize(), 1 );

    // See if we can already forward the data to the local connection
    COMCORE::CTCPClientSocket* localClientConnection = GetLocalConnectionForRemoteConnection( remoteConnection->GetSocketID() );
    if ( GUCEF_NULL != localClientConnection )
    {
        // send all data from the data buffer
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Forwarding received " + CORE::UInt32ToString( buffer.GetBufferedDataSizeInBytes() ) + " bytes of data from remote SPE server to actual server" );
        SendAllDataInBuffer( buffer, *localClientConnection );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToRemoteSPEDataSent( CORE::CNotifier* notifier    ,
                                                        const CORE::CEvent& eventid  ,
                                                        CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ServerPortExtenderClient: Sending data to remote SPE server" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnClientToRemoteSPESocketError( CORE::CNotifier* notifier    ,
                                                           const CORE::CEvent& eventid  ,
                                                           CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    CORE::Int32 socketErrorCode = static_cast< COMCORE::CTCPClientSocket::TSocketErrorEventData* >( eventdata )->GetData();
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ServerPortExtenderClient: socket error on a connection to the remote SPE server: " + CORE::Int32ToString( socketErrorCode ) );

    // make sure we dismantle our connection links
    RemoveConnectionUsingSPEClient( static_cast< COMCORE::CTCPClientSocket* >( notifier ) );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnControlMsg( TServerPortExtenderProtocolEnum msgType ,
                                         const CORE::UInt8* data                 )
{GUCEF_TRACE;

    switch ( msgType )
    {
        case SPEP_INITIAL_CONTROL_SERVER_MSG :
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Received initial handshake on control connection" );
            m_controlConnectionInitialized = true;
            break;
        }
        case SPEP_CONTROL_SERVER_MSG_CREATE_CONNECTION_REQUEST :
        {
            if ( m_controlConnectionInitialized )
            {
                // initiate a new connection to the SPE server
                // we create the client socket and add it to our list of connections
                COMCORE::CTCPClientSocket* clientSocket = new COMCORE::CTCPClientSocket( m_pulseGenerator, false );
                clientSocket->ConnectTo( m_remoteSPEReversedServer );

                // Subscribe to client events
                TEventCallback callback( this, &CServerPortExtenderClient::OnClientToRemoteSPEConnected );
                SubscribeTo( clientSocket                              ,
                             COMCORE::CTCPClientSocket::ConnectedEvent ,
                             callback                                  );
                TEventCallback callback2( this, &CServerPortExtenderClient::OnClientToRemoteSPEDisconnected );
                SubscribeTo( clientSocket                                 ,
                             COMCORE::CTCPClientSocket::DisconnectedEvent ,
                             callback2                                    );
                TEventCallback callback3( this, &CServerPortExtenderClient::OnClientToRemoteSPEDataSent );
                SubscribeTo( clientSocket                             ,
                             COMCORE::CTCPClientSocket::DataSentEvent ,
                             callback3                                );
                TEventCallback callback4( this, &CServerPortExtenderClient::OnClientToRemoteSPESocketError );
                SubscribeTo( clientSocket                                ,
                             COMCORE::CTCPClientSocket::SocketErrorEvent ,
                             callback4                                   );
                TEventCallback callback5( this, &CServerPortExtenderClient::OnClientToRemoteSPEDataRecieved );
                SubscribeTo( clientSocket                                 ,
                             COMCORE::CTCPClientSocket::DataRecievedEvent ,
                             callback5                                    );

                m_rsClientConnections[ clientSocket ];

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Received request for a new connection to remote SPE server, created client and attempting to connect" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Received request for new connection on an uninitialized control connection" );
            }
            break;
        }
        default:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Received control message of unknown type" );
        }
    }

}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnControlClientConnected( CORE::CNotifier* notifier    ,
                                                     const CORE::CEvent& eventid  ,
                                                     CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Control connection established, sending initial handshake,.." );

    COMCORE::CTCPClientSocket* clientSocket = static_cast< COMCORE::CTCPClientSocket* >( notifier );

    // Send welcome handshake with protocol version so the server can determine
    // whether it is compatible with this client

    char msg[ 3 ];
    msg[ 0 ] = (char) SPEP_INITIAL_CONTROL_CLIENT_MSG;
    msg[ 1 ] = 45; // magic number, additional sanity check
    msg[ 2 ]= SPEP_PROTOCOL_VERSION;
    clientSocket->Send( msg, 3 );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnControlClientDisconnected( CORE::CNotifier* notifier    ,
                                                        const CORE::CEvent& eventid  ,
                                                        CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Control connection disconnected" );

    // Make sure the control connection is always up and running unless explicitly asked to disconnect
    if ( !m_disconnectRequested )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Starting control connection reconnect timer with interval " + CORE::UInt32ToString( m_reconnectTimer.GetInterval() ) );
        m_reconnectTimer.SetEnabled( true );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnControlClientDataRecieved( CORE::CNotifier* notifier    ,
                                                        const CORE::CEvent& eventid  ,
                                                        CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Data received on the control connection" );

    // Get the data buffer
    COMCORE::CTCPClientSocket::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
    const CORE::CDynamicBuffer& data = eData->GetData();

    CORE::UInt32 dataSize = data.GetDataSize();
    if ( dataSize > 0 )
    {
        TServerPortExtenderProtocolEnum msgType = (TServerPortExtenderProtocolEnum) data.AsConstType< CORE::Int8 >( 0 );
        const CORE::UInt8* dataPtr = dataSize > 1 ? (const CORE::UInt8*) data.GetConstBufferPtr( 1 ) : GUCEF_NULL;
        OnControlMsg( msgType, dataPtr );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnControlClientDataSent( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventid  ,
                                                    CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "ServerPortExtenderClient: Data send on the control connection" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::OnControlClientSocketError( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventid  ,
                                                       CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    CORE::Int32 socketErrorCode = static_cast< COMCORE::CTCPClientSocket::TSocketErrorEventData* >( eventdata )->GetData();
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ServerPortExtenderClient: Socket error on the control connection: " + CORE::Int32ToString( socketErrorCode ) );

    // Make sure the control connection is always up and running unless explicitly asked to disconnect
    if ( !m_disconnectRequested )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Starting control connection reconnect timer with interval " + CORE::UInt32ToString( m_reconnectTimer.GetInterval() ) );

        m_reconnectTimer.SetEnabled( true );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtenderClient::MapLocalToRemoteConnection( COMCORE::CTCPClientSocket* localSocket  ,
                                                       COMCORE::CTCPClientSocket* remoteSocket )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Mapped connection to the actual server to a connection to the remote SPE server" );

    m_remoteToLocalConnectionMap[ remoteSocket->GetSocketID() ] = localSocket;
    m_localToRemoteConnectionMap[ localSocket->GetSocketID() ] = remoteSocket;
}

/*-------------------------------------------------------------------------*/

COMCORE::CTCPClientSocket*
CServerPortExtenderClient::GetRemoteConnectionForLocalConnection( const CORE::UInt32 socketId )
{GUCEF_TRACE;

    TSocketIdMap::iterator i = m_localToRemoteConnectionMap.find( socketId );
    if ( i != m_localToRemoteConnectionMap.end() )
    {
        return (*i).second;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

COMCORE::CTCPClientSocket*
CServerPortExtenderClient::GetLocalConnectionForRemoteConnection( const CORE::UInt32 socketId )
{GUCEF_TRACE;

    TSocketIdMap::iterator i = m_remoteToLocalConnectionMap.find( socketId );
    if ( i != m_remoteToLocalConnectionMap.end() )
    {
        return (*i).second;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/
