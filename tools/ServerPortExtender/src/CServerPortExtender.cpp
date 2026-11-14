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

#include "CServerPortExtender.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CServerPortExtender::ControlSocketOpenedEvent = "CServerPortExtender::ControlSocketOpenedEvent";
const CORE::CEvent CServerPortExtender::ReversedSocketOpenedEvent = "CServerPortExtender::ReversedSocketOpenedEvent";
const CORE::CEvent CServerPortExtender::ClientSocketOpenedEvent = "CServerPortExtender::ClientSocketOpenedEvent";
const CORE::CEvent CServerPortExtender::ControlConnectionEstablishedEvent = "CServerPortExtender::ControlConnectionEstablishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CServerPortExtender::RegisterEvents( void )
{GUCEF_TRACE;
    
    ControlSocketOpenedEvent.Initialize();
    ReversedSocketOpenedEvent.Initialize();
    ClientSocketOpenedEvent.Initialize();
    ControlConnectionEstablishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServerPortExtender::CServerPortExtender( CORE::PulseGeneratorPtr pulseGenerator )
    : CObservingNotifier()                                   ,
      m_reversedServerControlSocket( pulseGenerator, false ) ,
      m_reversedServerSocket( pulseGenerator, false )        ,
      m_serverSocket( pulseGenerator, false )                ,
      m_controlConnection( GUCEF_NULL )                            ,
      m_cConnectionBuffers()                                 ,
      m_rsConnectionBuffers()                                ,
      m_unmappedClientConnections()                          ,
      m_serverPort( 10234 )                                  ,
      m_reversedServerPort( 10235 )                          ,
      m_reversedServerControlPort( 10236 )                   ,
      m_remoteToLocalConnectionMap()                         ,
      m_controlConnectionInitialized( false )
{GUCEF_TRACE;

    RegisterEvents();

    SubscribeTo( &m_reversedServerControlSocket );
    SubscribeTo( &m_reversedServerSocket );
    SubscribeTo( &m_serverSocket );
    
    TEventCallback callback( this, &CServerPortExtender::OnPulse );
    SubscribeTo( pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent ,
                 callback                          );
                 
    pulseGenerator->RequestPeriodicPulses( this, 10 );
}

/*-------------------------------------------------------------------------*/

CServerPortExtender::~CServerPortExtender()
{GUCEF_TRACE;
    
    Disconnect();
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::Disconnect( void )
{GUCEF_TRACE;

    m_serverSocket.Close();
    m_reversedServerSocket.Close();
    m_reversedServerControlSocket.Close();
    
    m_controlConnectionInitialized = false;
    m_controlConnection = GUCEF_NULL;
    
    m_cConnectionBuffers.clear();
    m_rsConnectionBuffers.clear();
    m_unmappedClientConnections.clear();
    m_remoteToLocalConnectionMap.clear();
}

/*-------------------------------------------------------------------------*/

bool
CServerPortExtender::IsControlConnectionInitialized( void ) const
{GUCEF_TRACE;

    return m_controlConnectionInitialized;
}

/*-------------------------------------------------------------------------*/

bool
CServerPortExtender::ListenForClientsOnPort( CORE::UInt16 port )
{GUCEF_TRACE;

    m_serverPort = port;
    return m_serverSocket.ListenOnPort( m_serverPort );
}

/*-------------------------------------------------------------------------*/
    
bool
CServerPortExtender::ListenForReversedControlClientOnPort( CORE::UInt16 port )
{GUCEF_TRACE;

    m_reversedServerControlPort = port;
    return m_reversedServerControlSocket.ListenOnPort( m_reversedServerControlPort );
}

/*-------------------------------------------------------------------------*/
    
bool
CServerPortExtender::ListenForReversedClientsOnPort( CORE::UInt16 port )
{GUCEF_TRACE;

    m_reversedServerPort = port;
    return m_reversedServerSocket.ListenOnPort( m_reversedServerPort );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnPulse( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventid  ,
                              CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // transfer data buffered from local clients to the remote SPE client
    CORE::CDynamicBuffer transferBuffer;    
    TConnectionBuffers::iterator i = m_cConnectionBuffers.begin();
    while ( i != m_cConnectionBuffers.end() )
    {
        // Check to see if we need to transfer data for this connection to its linked connection
        CORE::CCyclicDynamicBuffer& cBuffer = (*i).second;
        if ( cBuffer.HasBufferedData() )
        {        
            // Since we have data to transfer check if the corresponding connection is available for transfer
            COMCORE::CTCPServerConnection* serverConnection = GetRemoteConnectionForLocalConnection( (*i).first->GetSocketID() );
            if ( GUCEF_NULL != serverConnection )
            {
                // transfer the data
                CORE::UInt32 bytesRead;
                do
                {        
                    bytesRead = (*i).second.ReadBlockTo( transferBuffer );                
                    if ( bytesRead > 0 )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Fowarding buffered client data to remote SPE client" );                        
                        serverConnection->Send( transferBuffer.GetConstBufferPtr(), transferBuffer.GetDataSize() );
                    }
                }
                while ( bytesRead > 0 );
            }
        }        
        ++i;
    }
    
    // transfer data buffered from the remote SPE client to local clients
    i = m_rsConnectionBuffers.begin();
    while ( i != m_rsConnectionBuffers.end() )
    {
        // Check to see if we need to transfer data for this connection to its linked connection
        CORE::CCyclicDynamicBuffer& cBuffer = (*i).second;
        if ( cBuffer.HasBufferedData() )
        {        
            // Since we have data to transfer check if the corresponding connection is available for transfer
            COMCORE::CTCPServerConnection* serverConnection = GetLocalConnectionForRemoteConnection( (*i).first->GetSocketID() );
            if ( GUCEF_NULL != serverConnection )
            {
                // transfer the data
                CORE::UInt32 bytesRead;
                do
                {        
                    bytesRead = (*i).second.ReadBlockTo( transferBuffer );                
                    if ( bytesRead > 0 )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Fowarding buffered remote SPE client data to local client" );
                        serverConnection->Send( transferBuffer.GetConstBufferPtr(), transferBuffer.GetDataSize() );
                    }
                }
                while ( bytesRead > 0 );
            }
        }        
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSControlClientConnected( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventid  ,
                                                 CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Sending initial handshake on control connection" );
    
    // Send welcome handshake with protocol version so the client can determine
    // whether it is compatible with this server
    
    char msg[ 3 ];
    msg[ 0 ] = (char) SPEP_INITIAL_CONTROL_SERVER_MSG;
    msg[ 1 ] = 45; // magic number, additional sanity check
    msg[ 2 ]= SPEP_PROTOCOL_VERSION;
    connection->Send( msg, 3 );
    
    // If we already have an existing control connection we should close it
    // we only support 1 control connection at a time.
    if ( GUCEF_NULL != m_controlConnection )
    {
        m_controlConnection->Close();
        m_controlConnection = GUCEF_NULL;
    }
    m_controlConnection = connection;
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSControlClientDisconnected( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventid  ,
                                                    CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Control connection has been disconnected" );

    m_controlConnectionInitialized = false;
    m_controlConnection = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSControlClientDataSent( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventid  ,
                                                CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Sending data on the control connection" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSControlClientSocketError( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventid  ,
                                                   CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ServerPortExtender: Socket error on the control connection" );
    
    m_controlConnectionInitialized = false;
    m_controlConnection = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnControlMsg( TServerPortExtenderProtocolEnum msgType ,
                                   const CORE::UInt8* data                 )
{GUCEF_TRACE;

    switch ( msgType )
    {
        case SPEP_INITIAL_CONTROL_CLIENT_MSG :
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtenderClient: Received initial handshake on control connection" );
            m_controlConnectionInitialized = true;
            NotifyObservers( ControlConnectionEstablishedEvent );
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
CServerPortExtender::OnRSControlClientDataRecieved( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventid  ,
                                                    CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Received data on the control connection" );
    
    // Get the data buffer
    COMCORE::CTCPConnection::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventdata );
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
CServerPortExtender::OnRSClientConnected( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventid  ,
                                          CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // We won't do anything with this incoming rs connection until we get our initial
    // message which will tell us how to setup the routing for this connection
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Reversed server client connection established" );
    COMCORE::CTCPServerConnection* rsConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    if ( !MapRemoteConnectionToLocalConnection( rsConnection ) )
    {
        // unable to map this reversed connection to a waiting client connection
        rsConnection->Close();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Unable to mapped a client connection to a reversed SPE connection, perhaps the client for which the connection was requested has already disconnected again" );
    }
}

/*-------------------------------------------------------------------------*/

COMCORE::CTCPServerConnection*
CServerPortExtender::GetLocalConnectionForRemoteConnection( CORE::UInt32 rsSocketId )
{GUCEF_TRACE;

    TConnectionMap::iterator i = m_remoteToLocalConnectionMap.find( rsSocketId );
    if ( i != m_remoteToLocalConnectionMap.end() )
    {
        return m_serverSocket.GetConnectionBySocketId( (*i).second );
    }
    return GUCEF_NULL; 
}

/*-------------------------------------------------------------------------*/

COMCORE::CTCPServerConnection*
CServerPortExtender::GetRemoteConnectionForLocalConnection( CORE::UInt32 localSocketId )
{GUCEF_TRACE;

    TConnectionMap::iterator i = m_remoteToLocalConnectionMap.begin();
    while ( i != m_remoteToLocalConnectionMap.end() )
    {
        if ( (*i).second == localSocketId )
        {
            return m_reversedServerSocket.GetConnectionBySocketId( (*i).first );
        }
        ++i;
    }
    return GUCEF_NULL; 
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSClientDisconnected( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventid  ,
                                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // Since the remote link has been severed we should sever the local link since its
    // the same virtual socket connection    
    COMCORE::CTCPServerConnection* rsConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    COMCORE::CTCPServerConnection* lConnection = GetLocalConnectionForRemoteConnection( rsConnection->GetSocketID() );
    if ( GUCEF_NULL != lConnection )
    {
        // the local connection still exists,... close it
        lConnection->Close();
    }
    
    // Remove the buffer for this connection
    m_rsConnectionBuffers.erase( rsConnection );
    m_remoteToLocalConnectionMap.erase( rsConnection->GetSocketID() );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSClientDataSent( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventid  ,
                                         CORE::CICloneable* eventdata )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSClientSocketError( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventid  ,
                                            CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ServerPortExtender: Socket error on a reversed client-server connection" );
}

/*-------------------------------------------------------------------------*/

bool
CServerPortExtender::MapRemoteConnectionToLocalConnection( COMCORE::CTCPServerConnection* rsConnection )
{GUCEF_TRACE;
    
    if ( m_unmappedClientConnections.size() > 0 )
    {
        TConnectionSet::iterator i = m_unmappedClientConnections.begin();
        CORE::UInt32 localClientSocketId = (*i)->GetSocketID();
        m_remoteToLocalConnectionMap[ rsConnection->GetSocketID() ] = localClientSocketId;
        m_unmappedClientConnections.erase( i );
        
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Mapped a client connection to a reversed SPE connection" );
        
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnRSClientDataRecieved( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventid  ,
                                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Received data on a reversed client-server connection" );
    
    COMCORE::CTCPServerConnection* rsConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    COMCORE::CTCPServerConnection* lConnection = GetLocalConnectionForRemoteConnection( rsConnection->GetSocketID() );
    if ( GUCEF_NULL != lConnection )
    {
        // write data to the cyclic buffer and process the data transfer to the local client connection        
        COMCORE::CTCPServerConnection::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPServerConnection::TDataRecievedEventData* >( eventdata );
        const CORE::CDynamicBuffer& data = eData->GetData();
        CORE::CCyclicDynamicBuffer& cyclicBuffer = m_rsConnectionBuffers[ rsConnection ];
        
        // First we write the data to the buffer to assure propper data ordering
        cyclicBuffer.Write( data.GetConstBufferPtr(), data.GetDataSize(), 1 );
        
        // now we process all data in the buffer
        CORE::CDynamicBuffer readBuffer;
        while ( 0 != cyclicBuffer.ReadBlockTo( readBuffer ) )
        {
            // Send the block
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Forwarding data from a reversed client-server connection to a client" );
            lConnection->Send( readBuffer.GetConstBufferPtr(), readBuffer.GetDataSize() );
            readBuffer.SetDataSize( 0 );
        }
    }
    else
    {
        // since we cannot locate the local connection we will hook this connection up to an unmapped client connection 
        if ( !MapRemoteConnectionToLocalConnection( rsConnection ) )
        {
            // Unable to map this connection, it is useless
            rsConnection->Close();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnClientConnected( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventid  ,
                                        CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // If a client connects to our extended port we have to make sure 
    // it triggers the remote SPE client to establish a connection in the 
    // opposite direction to this SPE server and establish a connection to 
    // the actual remote server port
    
    COMCORE::CTCPServerConnection* clientConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    
    // Initial sanity check
    if ( GUCEF_NULL == m_controlConnection )
    {
        // Nothing we can do with this connection
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Client connection established without a control connection, unable to proceed,.. disconnecting client" );
        clientConnection->Close();
        return;
    }

    // Send the request for a new connection to be established ASAP  
    char msg[ 1 ];
    msg[ 0 ] = (char) SPEP_CONTROL_SERVER_MSG_CREATE_CONNECTION_REQUEST;
    m_controlConnection->Send( msg, 1 );
    
    // Reserve a buffer for this connection.
    // We need to buffer the different end of the connection since data might 
    // already be send before we complete the loop
    m_cConnectionBuffers[ clientConnection ].Clear( true );
    
    // Put this connection in this set of unmapped connections
    m_unmappedClientConnections.insert( clientConnection );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnClientDisconnected( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventid  ,
                                           CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: A client disconnected" );
    
    // If a local client disconnects we have to disconnect the other half of the
    // virtual connection if it is still active, meaning the connection
    // from this server to the remote client
    
    COMCORE::CTCPServerConnection* clientConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );    
    COMCORE::CTCPServerConnection* rsConnection = GetRemoteConnectionForLocalConnection( clientConnection->GetSocketID() );
    if ( GUCEF_NULL != rsConnection )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Closing reversed client-server connection for a client who disconnected" );
        
        m_remoteToLocalConnectionMap.erase( rsConnection->GetSocketID() );
        rsConnection->Close();
    }
    
    // Remove the buffer for this connection
    m_cConnectionBuffers.erase( clientConnection );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnClientDataSent( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventid  ,
                                       CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ServerPortExtender: Sent data to client" );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnClientSocketError( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventid  ,
                                          CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ServerPortExtender: There was a socket error on a client connection" );
    
    COMCORE::CTCPServerConnection* clientConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );    
    COMCORE::CTCPServerConnection* rsConnection = GetRemoteConnectionForLocalConnection( clientConnection->GetSocketID() );
    if ( GUCEF_NULL != rsConnection )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Closing reversed client-server connection for a client who a client had a socket error" );
        
        m_remoteToLocalConnectionMap.erase( rsConnection->GetSocketID() );
        rsConnection->Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnClientDataRecieved( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventid  ,
                                           CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Received data from a client" );
    
    COMCORE::CTCPServerConnection* lConnection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    COMCORE::CTCPServerConnection* rsConnection = GetRemoteConnectionForLocalConnection( lConnection->GetSocketID() );
    
    // write data to the cyclic buffer and process the data transfer to the local client connection        
    COMCORE::CTCPServerConnection::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPServerConnection::TDataRecievedEventData* >( eventdata );
    const CORE::CDynamicBuffer& data = eData->GetData();
    CORE::CCyclicDynamicBuffer& cyclicBuffer = m_cConnectionBuffers[ lConnection ];
    
    // First we write the data to the buffer to assure propper data ordering
    cyclicBuffer.Write( data.GetConstBufferPtr(), data.GetDataSize(), 1 );
    
    // Check to see if we have the connection to the remote SPE client established
    // If so we can proceed with sending all the data
    if ( GUCEF_NULL != rsConnection )
    {
        // now we process all data in the buffer
        CORE::CDynamicBuffer readBuffer;
        while ( 0 != cyclicBuffer.ReadBlockTo( readBuffer ) )
        {
            // Send the block
            rsConnection->Send( readBuffer.GetConstBufferPtr(), readBuffer.GetDataSize() );
            readBuffer.SetDataSize( 0 );
            
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Fowarded client data to mapped reverse-client-server connection" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnReversedServerControlSocketNotify( CORE::CNotifier* notifier    ,
                                                          const CORE::CEvent& eventid  ,
                                                          CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // this is where the remote connection comes in that will carry all the control
    // commands to operate the reversed client-server tunnel

    if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventid )
    {
        // Now that the control connection is established we can listen for clients on our extended server port
        if ( !m_serverSocket.IsActive() )
        {
            m_serverSocket.ListenOnPort( m_serverPort );
        }
        
        // Get the connection object
        COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata );
        COMCORE::CTCPServerSocket::TConnectionInfo& connectionInfo = eData->GetData();

        // Subscribe to connection events        
        TEventCallback callback( this, &CServerPortExtender::OnRSControlClientConnected );
        SubscribeTo( connectionInfo.connection                     ,
                     COMCORE::CTCPServerConnection::ConnectedEvent ,
                     callback                                      );
        TEventCallback callback2( this, &CServerPortExtender::OnRSControlClientDisconnected );
        SubscribeTo( connectionInfo.connection                        ,
                     COMCORE::CTCPServerConnection::DisconnectedEvent ,
                     callback2                                        );
        TEventCallback callback3( this, &CServerPortExtender::OnRSControlClientDataSent );
        SubscribeTo( connectionInfo.connection                    ,
                     COMCORE::CTCPServerConnection::DataSentEvent ,
                     callback3                                    );
        TEventCallback callback4( this, &CServerPortExtender::OnRSControlClientSocketError );
        SubscribeTo( connectionInfo.connection                       ,
                     COMCORE::CTCPServerConnection::SocketErrorEvent ,
                     callback4                                       );
        TEventCallback callback5( this, &CServerPortExtender::OnRSControlClientDataRecieved );
        SubscribeTo( connectionInfo.connection                        ,
                     COMCORE::CTCPServerConnection::DataRecievedEvent ,
                     callback5                                        );

       OnRSControlClientConnected( connectionInfo.connection, COMCORE::CTCPConnection::ConnectedEvent, GUCEF_NULL );
    }
    else
    if ( COMCORE::CTCPServerSocket::ClientDisconnectedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Client disconnected from server control socket" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ClientErrorEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Client error on server control socket" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketOpenedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Server control socket opened,.. waiting for ServerPortExtenderClient control connection" );
        
        NotifyObservers( ControlSocketOpenedEvent );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketClosedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: server control socket closed" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketErrorEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Socket error on server control socket" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketClientErrorEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Server socket client error on server control socket" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketMaxConnectionsChangedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Max connections changed on server control socket" );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnReversedServerSocketNotify( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventid  ,
                                                   CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // this is where the remote connection comes in to establish the reversed
    // client-server connnection. This is the connection we have to keep alive to
    // allow reversed client-server tunneling

    if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Reversed connection established between SPE server and client" );
        
        if ( !m_serverSocket.IsActive() )
        {
            m_serverSocket.ListenOnPort( m_serverPort );
        }
        
        COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata );
        COMCORE::CTCPServerSocket::TConnectionInfo& connectionInfo = eData->GetData();
        
        TEventCallback callback( this, &CServerPortExtender::OnRSClientConnected );
        SubscribeTo( connectionInfo.connection                     ,
                     COMCORE::CTCPServerConnection::ConnectedEvent ,
                     callback                                      );
        TEventCallback callback2( this, &CServerPortExtender::OnRSClientDisconnected );
        SubscribeTo( connectionInfo.connection                        ,
                     COMCORE::CTCPServerConnection::DisconnectedEvent ,
                     callback2                                        );
        TEventCallback callback3( this, &CServerPortExtender::OnRSClientDataSent );
        SubscribeTo( connectionInfo.connection                    ,
                     COMCORE::CTCPServerConnection::DataSentEvent ,
                     callback3                                    );
        TEventCallback callback4( this, &CServerPortExtender::OnRSClientSocketError );
        SubscribeTo( connectionInfo.connection                       ,
                     COMCORE::CTCPServerConnection::SocketErrorEvent ,
                     callback4                                       );
        TEventCallback callback5( this, &CServerPortExtender::OnRSClientDataRecieved );
        SubscribeTo( connectionInfo.connection                        ,
                     COMCORE::CTCPServerConnection::DataRecievedEvent ,
                     callback5                                        );

        OnRSClientConnected( connectionInfo.connection, COMCORE::CTCPServerConnection::ConnectedEvent, GUCEF_NULL );                   
    }
    else
    if ( COMCORE::CTCPServerSocket::ClientDisconnectedEvent == eventid )
    {
    }
    else
    if ( COMCORE::CTCPServerSocket::ClientErrorEvent == eventid )
    {
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketOpenedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Reversed server socket opened,.. waiting for ServerPortExtenderClient" );
        
        NotifyObservers( ReversedSocketOpenedEvent );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketClosedEvent == eventid )
    {
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketErrorEvent == eventid )
    {
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketClientErrorEvent == eventid )
    {
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketMaxConnectionsChangedEvent == eventid )
    {
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnServerSocketClientConnected( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventid  ,
                                                    CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: A client connected to the extended server port" );
    
    // get the connection info
    COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata );
    COMCORE::CTCPServerSocket::TConnectionInfo& connectionInfo = eData->GetData();
    
    // Check if we have a control connection up and running,.. if not then disconnect the client
    // because we will be unable to complete the connection to the remote server
    if ( !m_controlConnectionInitialized )
    {
        connectionInfo.connection->Close();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Disconnecting client because no control connection is available" );
        return;
    }    
    
    // subscribe the handlers to the connection object
    TEventCallback callback( this, &CServerPortExtender::OnClientConnected );
    SubscribeTo( connectionInfo.connection                     ,
                 COMCORE::CTCPServerConnection::ConnectedEvent ,
                 callback                                      );
    TEventCallback callback2( this, &CServerPortExtender::OnClientDisconnected );
    SubscribeTo( connectionInfo.connection                        ,
                 COMCORE::CTCPServerConnection::DisconnectedEvent ,
                 callback2                                        );
    TEventCallback callback3( this, &CServerPortExtender::OnClientDataSent );
    SubscribeTo( connectionInfo.connection                    ,
                 COMCORE::CTCPServerConnection::DataSentEvent ,
                 callback3                                    );
    TEventCallback callback4( this, &CServerPortExtender::OnClientSocketError );
    SubscribeTo( connectionInfo.connection                       ,
                 COMCORE::CTCPServerConnection::SocketErrorEvent ,
                 callback4                                       );
    TEventCallback callback5( this, &CServerPortExtender::OnClientDataRecieved );
    SubscribeTo( connectionInfo.connection                        ,
                 COMCORE::CTCPServerConnection::DataRecievedEvent ,
                 callback5                                        );

    OnClientConnected( connectionInfo.connection, COMCORE::CTCPServerConnection::ConnectedEvent, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnServerSocketNotify( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventid  ,
                                           CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // This is where clients connect that would normally connect to the remote
    // server port.
    
    if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventid )
    {
        OnServerSocketClientConnected( notifier, eventid, eventdata );
    }
    else
    if ( COMCORE::CTCPServerSocket::ClientDisconnectedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: A client disconnected from the extended server port" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ClientErrorEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: A client error occured with a client connected to the extended server port" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketOpenedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Extended server socket opened,.. listning for clients" );
        NotifyObservers( ClientSocketOpenedEvent );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketClosedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Extended server socket closed" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketErrorEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Socket error occured in the extended server socket" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketClientErrorEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Client error occured on the extended server socket" );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketMaxConnectionsChangedEvent == eventid )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServerPortExtender: Maximum number of connections changed on the extended server socket" );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerPortExtender::OnNotify( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventid  ,
                               CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( notifier == &m_reversedServerControlSocket )
    {
        OnReversedServerControlSocketNotify( notifier  ,
                                             eventid   ,
                                             eventdata );
    }
    else
    if ( notifier == &m_reversedServerSocket )
    {
        OnReversedServerSocketNotify( notifier  ,
                                      eventid   ,
                                      eventdata );
    }
    else
    if ( notifier == &m_serverSocket )
    {
        OnServerSocketNotify( notifier  ,
                              eventid   ,
                              eventdata );
    }
}

/*-------------------------------------------------------------------------*/