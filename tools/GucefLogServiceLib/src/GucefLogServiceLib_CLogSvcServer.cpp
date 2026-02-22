/*
 *  GucefLogServiceLib: Library containing the main logic for the Logging service
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H
#include "GucefLogServiceLib_CILogSvcServerLogger.h"
#define GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H
#endif /* GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H ? */

#ifndef GUCEF_LOGSERVICELIB_PROTOCOL_H
#include "GucefLogServiceLib_protocol.h"
#define GUCEF_LOGSERVICELIB_PROTOCOL_H
#endif /* GUCEF_LOGSERVICELIB_PROTOCOL_H ? */

#ifndef GUCEF_LOGSERVICELIB_CCLIENTINITMESSAGE_H
#include "GucefLogServiceLib_CClientInitMessage.h"
#define GUCEF_LOGSERVICELIB_CCLIENTINITMESSAGE_H
#endif /* GUCEF_LOGSERVICELIB_CCLIENTINITMESSAGE_H ? */

#include "GucefLogServiceLib_CLogSvcServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace LOGSERVICELIB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

//                              1024 *  1KB = 1MB * 10 = 10MB
////10 * 1024 * 1024
#define MAX_ACCEPTED_MSGLENGTH  2048

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CLogSvcServer::CLogSvcServer( void )
    : CORE::CObservingNotifier()                                               ,
      m_tcpServer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false ) ,
      m_clientInfoMap()                                                        ,
      m_loggers()
{GUCEF_TRACE;

    RegisterServerSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

CLogSvcServer::CLogSvcServer( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()           ,
      m_tcpServer( pulseGenerator, false ) ,
      m_clientInfoMap()                    ,
      m_loggers()
{GUCEF_TRACE;

    RegisterServerSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::RegisterServerSocketEventHandlers( void )
{
    TEventCallback callback( this, &CLogSvcServer::OnServerSocketClientConnected );
    SubscribeTo( &m_tcpServer                                    ,
                 COMCORE::CTCPServerSocket::ClientConnectedEvent ,
                 callback                                        );
    TEventCallback callback2( this, &CLogSvcServer::OnServerSocketClientDisconnected );
    SubscribeTo( &m_tcpServer                                       ,
                 COMCORE::CTCPServerSocket::ClientDisconnectedEvent ,
                 callback2                                          );
    TEventCallback callback3( this, &CLogSvcServer::OnServerSocketClientError );
    SubscribeTo( &m_tcpServer                                ,
                 COMCORE::CTCPServerSocket::ClientErrorEvent ,
                 callback3                                   );
    TEventCallback callback4( this, &CLogSvcServer::OnServerSocketServerSocketClosed );
    SubscribeTo( &m_tcpServer                                       ,
                 COMCORE::CTCPServerSocket::ServerSocketClosedEvent ,
                 callback4                                          );
    TEventCallback callback5( this, &CLogSvcServer::OnServerSocketServerSocketError );
    SubscribeTo( &m_tcpServer                                      ,
                 COMCORE::CTCPServerSocket::ServerSocketErrorEvent ,
                 callback5                                         );
    TEventCallback callback6( this, &CLogSvcServer::OnServerSocketServerSocketClientError );
    SubscribeTo( &m_tcpServer                                            ,
                 COMCORE::CTCPServerSocket::ServerSocketClientErrorEvent ,
                 callback6                                               );
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::RegisterClientConnectionEventHandlers( COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    TEventCallback callback( this, &CLogSvcServer::OnClientConnectionDataReceived );
    SubscribeTo( connection                                       ,
                 COMCORE::CTCPServerConnection::DataRecievedEvent ,
                 callback                                         );

    //static const CORE::CEvent ConnectedEvent;
    //static const CORE::CEvent DisconnectedEvent;
    //static const CORE::CEvent DataSentEvent;
    //static const CORE::CEvent SocketErrorEvent;
}

/*-------------------------------------------------------------------------*/

CLogSvcServer::~CLogSvcServer()
{GUCEF_TRACE;

    m_tcpServer.Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLogSvcServer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::LOGSERVICELIB::CLogSvcServer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::RegisterLogger( CILogSvcServerLogger* logger )
{GUCEF_TRACE;

    m_loggers.insert( logger );
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::UnregisterLogger( CILogSvcServerLogger* logger )
{GUCEF_TRACE;

    m_loggers.erase( logger );
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcServer::ListenOnPort( CORE::UInt16 port )
{GUCEF_TRACE;

    return m_tcpServer.ListenOnPort( port );
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::ProcessReceivedMessage( TClientInfo& clientInfo                   ,
                                       COMCORE::CTCPServerConnection* connection ,
                                       const CORE::CDynamicBuffer& messageBuffer )
{GUCEF_TRACE;

    try
    {
        // First read the message type and handle accordingly
        CORE::UInt8 msgType = messageBuffer.AsConstType< CORE::UInt8 >( 0 );
        switch ( msgType )
        {
            case LOGSVCMSGTYPE_CLIENTINFO:
            {
                CClientInitMessage initMessage;
                if ( initMessage.ReadFromBuffer( messageBuffer, false ) )
                {
                    clientInfo.logClientVersion = initMessage.GetClientVersion();
                    clientInfo.appName = initMessage.GetApplicationName();
                    clientInfo.processId = initMessage.GetProcessId();
                    clientInfo.processName = initMessage.GetProcessName();

                    // Set the client's address and port for easy unique addressability
                    clientInfo.addressAndPort = connection->GetRemoteIP().AddressAndPortAsString();

                    // If we got here without an exception then the we successfully received all info required to
                    // consider the connection as initialized.
                    clientInfo.initialized = true;

                    // tell the loggers we have a new client for which we can begin logging
                    TLoggerList::iterator i = m_loggers.begin();
                    while ( i != m_loggers.end() )
                    {
                        (*i)->StartOfLoggingForClient( clientInfo );
                        ++i;
                    }

                    // Send notification to the client that the initialization was successfull
                    CORE::UInt8 statusCode = LOGSVCMSGTYPE_INITIALIZED;
                    connection->Send( &statusCode, 1 );
                }
                else
                {
                    // Something is wrong with this msg buffer, kill the connection
                    connection->Close();
                }
                break;
            }
            case LOGSVCMSGTYPE_FLUSHLOG:
            {
                // We are being asked to flush the log
                TLoggerList::iterator i = m_loggers.begin();
                while ( i != m_loggers.end() )
                {
                    (*i)->FlushLog( clientInfo );
                    ++i;
                }
                break;
            }
            case LOGSVCMSGTYPE_LOGMSG:
            {
                // This is what its all about, we have been given a log message.
                // Parse the log message from the transmission
                CORE::UInt32 offset = 1;
                CILogSvcServerLogger::TLogMsgType logMsgTypeValue = (CILogSvcServerLogger::TLogMsgType) messageBuffer.AsConstType< CORE::Int16 >( offset );
                offset += 2;
                CORE::Int32 logLevel = messageBuffer.AsConstType< CORE::Int32 >( offset );
                offset += 4;
                CORE::UInt32 threadId = messageBuffer.AsConstType< CORE::UInt32 >( offset );
                offset += 4;
                CORE::UInt64 unixEpochDt = messageBuffer.AsConstType< CORE::UInt64 >( offset );
                offset += 8;
                CORE::CTimestamp timestamp;
                timestamp.FromMillisecondsSinceEpoch( unixEpochDt );
                CORE::CString logMessage;
                CORE::UInt32 strLength = messageBuffer.GetDataSize() - offset;
                logMessage.Set( messageBuffer.AsConstTypePtr< char >( offset, strLength ), strLength );

                // Now send the log message to all loggers
                TLoggerList::iterator i = m_loggers.begin();
                while ( i != m_loggers.end() )
                {
                    (*i)->Log( clientInfo, logMsgTypeValue, logLevel, logMessage, threadId, timestamp );
                    ++i;
                }
                break;
            }
            default:
            {
                // We should not get here if the protocol is correctly adhered to and there is no
                // corruption in the stream. This is a fatal error for the connection.
                connection->Close();
            }
        }
    }
    catch ( CORE::CDynamicBuffer::EIllegalCast& )
    {
        // Something is wrong with this msg buffer, kill the connection
        connection->Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::ProcessReceivedData( TClientInfo& clientInfo                   ,
                                    COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    // Cycle as long as we have buffered data and no errors occur
    bool noError = true;
    while ( clientInfo.receiveBuffer.HasBufferedData() )
    {
        // First read the message delimiter if its fully available
        char msgDelimterHeader[ 5 ];
        if ( 5 == clientInfo.receiveBuffer.PeekElement( msgDelimterHeader, 5 ) )
        {
            if ( msgDelimterHeader[ 0 ] == LOGSVCMSGTYPE_DELIMITER )
            {
                // Get the total message length from the delimiter header
                CORE::UInt32 msgLength = 0;
                memcpy( &msgLength, msgDelimterHeader+1, 4 );

                // Perform a sanity check
                // Server side we limit the length of the message we will accept from the client
                if ( msgLength <= MAX_ACCEPTED_MSGLENGTH )
                {
                    // Check to see if the buffer actually has the data available already to form the complete message
                    if ( msgLength+5 <= clientInfo.receiveBuffer.GetBufferedDataSizeInBytes() )
                    {
                        // Since we already got he header data via a Peek() we will skip that data now
                        if ( 5 != clientInfo.receiveBuffer.SkipRead( 5 ) )
                        {
                            // This should not happen
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CLogSvcServer: Unable to Skip buffered data" );
                            connection->Close();
                            return;
                        }

                        // Make a buffer for this message and read it from the receive buffer
                        CORE::CDynamicBuffer messageBuffer( msgLength, false );
                        if ( msgLength == clientInfo.receiveBuffer.Read( messageBuffer.GetBufferPtr(), msgLength, 1 ) )
                        {
                            // Set the size of usefull data in the buffer
                            messageBuffer.SetDataSize( msgLength );

                            // Process this single message
                            ProcessReceivedMessage( clientInfo    ,
                                                    connection    ,
                                                    messageBuffer );
                        }
                        else
                        {
                            // This should not happen
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CLogSvcServer: Unable to Read() buffered data" );
                            connection->Close();
                            return;
                        }
                    }
                    else
                    {
                        // We don't have all the data yet to read this message, we will wait for more data to come in
                        return;
                    }
                }
                else
                {
                    // This connection is misbehaving
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CLogSvcServer: A message was received which exceeded the max msg size, disconnecting client" );
                    connection->Close();
                    return;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnClientConnectionDataReceived( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventid  ,
                                               CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // Get access to the event data
    COMCORE::CTCPServerConnection::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPServerConnection::TDataRecievedEventData* >( eventdata );
    const CORE::CDynamicBuffer& buffer = eData->GetData();

    // Get access to the connection object & client info
    COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    TClientInfo& clientInfo = m_clientInfoMap[ connection->GetConnectionIndex() ];

    // Handle the tcp stream, take into account message concatonation
    // First we copy the data recieved into our recieve buffer where we
    // can properly segment data because we retain whatever data we need
    if ( !( buffer.GetDataSize() == clientInfo.receiveBuffer.Write( buffer, 0 ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Failed to write all data recieved into a receive buffer for processsing" );
    }

    // Use the received data as a trigger to process whatever we can on this connection
    ProcessReceivedData( clientInfo ,
                         connection );
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnServerSocketClientConnected( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventid  ,
                                              CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // Get access to the event data
    COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata );
    COMCORE::CTCPServerSocket::TConnectionInfo& eventData = eData->GetData();

    // Build an initialization structure
    TClientInfo initialClientInfo;
    initialClientInfo.appName = "Unknown";
    initialClientInfo.processId = "Unknown";
    initialClientInfo.processName = "Unknown";
    initialClientInfo.logClientVersion = "Unknown";
    initialClientInfo.initialized = false;
    initialClientInfo.connected = true;

    m_clientInfoMap[ eventData.connectionIndex ] = initialClientInfo;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CLogSvcServer: Client connected from " + eventData.connection->GetRemoteIP().AddressAndPortAsString() );

    RegisterClientConnectionEventHandlers( eventData.connection );
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnServerSocketClientDisconnected( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventid  ,
                                                 CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // Get access to the event data
    COMCORE::CTCPServerSocket::TClientDisconnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientDisconnectedEventData* >( eventdata );
    COMCORE::CTCPServerSocket::TConnectionInfo& connectionInfo = eData->GetData().connectionInfo;

    TClientInfo& clientInfo = m_clientInfoMap[ connectionInfo.connectionIndex ];
    clientInfo.connected = false;

    // tell the loggers we should stop logging for this client
    TLoggerList::iterator i = m_loggers.begin();
    while ( i != m_loggers.end() )
    {
        (*i)->EndOfLoggingForClient( clientInfo );
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CLogSvcServer: Client from " + connectionInfo.hostAddress.HostnameAndPortAsString() + " disconnected" );
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnServerSocketClientError( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventid  ,
                                          CORE::CICloneable* eventdata )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnServerSocketServerSocketClosed( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventid  ,
                                                 CORE::CICloneable* eventdata )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnServerSocketServerSocketError( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventid  ,
                                                CORE::CICloneable* eventdata )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLogSvcServer::OnServerSocketServerSocketClientError( CORE::CNotifier* notifier    ,
                                                      const CORE::CEvent& eventid  ,
                                                      CORE::CICloneable* eventdata )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
