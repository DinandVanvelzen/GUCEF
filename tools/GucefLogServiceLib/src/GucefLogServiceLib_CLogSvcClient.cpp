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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */ 

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_LOGSERVICELIB_PROTOCOL_H
#include "GucefLogServiceLib_protocol.h"
#define GUCEF_LOGSERVICELIB_PROTOCOL_H
#endif /* GUCEF_LOGSERVICELIB_PROTOCOL_H ? */

#ifndef GUCEF_LOGSERVICELIB_CCLIENTINITMESSAGE_H
#include "GucefLogServiceLib_CClientInitMessage.h"
#define GUCEF_LOGSERVICELIB_CCLIENTINITMESSAGE_H
#endif /* GUCEF_LOGSERVICELIB_CCLIENTINITMESSAGE_H ? */

#include "GucefLogServiceLib_CLogSvcClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace LOGSERVICELIB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLogSvcClient::ClientVersion = "1.0";

const CORE::CEvent CLogSvcClient::ConnectingEvent = "GUCEF::LOGSERVICELIB::CLogSvcClient::ConnectingEvent";
const CORE::CEvent CLogSvcClient::ConnectedEvent = "GUCEF::LOGSERVICELIB::CLogSvcClient::ConnectedEvent";
const CORE::CEvent CLogSvcClient::DisconnectedEvent = "GUCEF::LOGSERVICELIB::CLogSvcClient::DisconnectedEvent";
const CORE::CEvent CLogSvcClient::SocketErrorEvent = "GUCEF::LOGSERVICELIB::CLogSvcClient::SocketErrorEvent";
const CORE::CEvent CLogSvcClient::ConnectionInitializedEvent = "GUCEF::LOGSERVICELIB::CLogSvcClient::ConnectionInitializedEvent";
const CORE::CEvent CLogSvcClient::IncompatibleWithServerEvent = "GUCEF::LOGSERVICELIB::CLogSvcClient::IncompatibleWithServerEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLogSvcClient::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectingEvent.Initialize();
    ConnectedEvent.Initialize();
    DisconnectedEvent.Initialize();
    SocketErrorEvent.Initialize();
    ConnectionInitializedEvent.Initialize();
    IncompatibleWithServerEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLogSvcClient::CLogSvcClient( void )
    : CLoggingTask()                   ,
      m_tcpClient( false )             ,
      m_appName( "Unknown" )           ,
      m_connectionInitialized( false )
{GUCEF_TRACE;

    RegisterEvents();

    SubscribeTo( &m_tcpClient );
}

/*-------------------------------------------------------------------------*/

CLogSvcClient::CLogSvcClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CLoggingTask()                       ,
      m_tcpClient( pulseGenerator, false ) ,
      m_appName( "Unknown" )               ,
      m_connectionInitialized( false )
{GUCEF_TRACE;

    RegisterEvents();

    SubscribeTo( &m_tcpClient );
}

/*-------------------------------------------------------------------------*/

CLogSvcClient::~CLogSvcClient()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::ConnectTo( const CORE::CString& address ,
                          CORE::UInt16 port            ,
                          bool blocking                )
{GUCEF_TRACE;

    Close();
    
    m_connectionInitialized = false;
    if ( m_tcpClient.ConnectTo( address, port, blocking ) )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::ConnectTo( const COMCORE::CHostAddress& address ,
                          bool blocking                        )
{GUCEF_TRACE;

    Close();
    
    m_connectionInitialized = false;
    if ( m_tcpClient.ConnectTo( address, blocking ) )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::Reconnect( bool blocking )
{GUCEF_TRACE;

    Close();
    
    m_connectionInitialized = false;
    if ( m_tcpClient.Reconnect( blocking ) )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcClient::Close( void )
{GUCEF_TRACE;

    if ( m_tcpClient.IsActive() )
    {
        m_connectionInitialized = false;
        m_tcpClient.Close();
    }
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::IsReadyToProcessCycle( void ) const
{
    return m_connectionInitialized;
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::OnTaskCycleLogWithoutFormatting( const CORE::TLogMsgType logMsgType ,
                                                const CORE::Int32 logLevel         ,
                                                const CORE::CString& logMessage    ,
                                                const CORE::UInt32 threadId        ,
                                                const CORE::CTimestamp& timestamp  )
{GUCEF_TRACE;

    return OnTaskCycleLog( logMsgType ,
                           logLevel   ,
                           logMessage ,
                           threadId   ,
                           timestamp  );
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::OnTaskCycleLog( const CORE::TLogMsgType logMsgType ,
                               const CORE::Int32 logLevel         ,
                               const CORE::CString& logMessage    ,
                               const CORE::UInt32 threadId        ,
                               const CORE::CTimestamp& timestamp  )
{GUCEF_TRACE;

    CORE::Int16 logMsgTypeValue = logMsgType;
    CORE::Int8 msgHeader[ 24 ];  // 24 = 1+4+1+2+4+4+8
    CORE::UInt32 logMsgLength = logMessage.Length() + 11;
    CORE::UInt64 unixEpochDt = timestamp.ToMillisecondsSinceEpoch(); 

    msgHeader[ 0 ] = (CORE::Int8) LOGSVCMSGTYPE_DELIMITER;   // set delimiter for message: 1 byte
    memcpy( msgHeader+1, &logMsgLength, 4 );                 // set the total message length : 4 bytes
    msgHeader[ 5 ] = (CORE::Int8) LOGSVCMSGTYPE_LOGMSG;      // set TCP msg type: 1 byte
    memcpy( msgHeader+6, &logMsgTypeValue, 2 );              // set log msg type: 2 bytes
    memcpy( msgHeader+8, &logLevel, 4 );                     // set log level: 4 bytes
    memcpy( msgHeader+12, &threadId, 4 );                    // set thread id: 4 bytes
    memcpy( msgHeader+16, &unixEpochDt, 8 );                 // set datetime as unix epoc offset in ms: 8 bytes

    if ( m_connectionInitialized )
    {
        // Send the logging msg header
        if ( m_tcpClient.Send( msgHeader, 24 ) )
        {
            // Now send the logging text
            m_tcpClient.Send( logMessage.C_String() ,
                              logMessage.Length()   );
        }
    }
    else
    {
        // The logging connection is not initialized yet
        // queue the msg until the connection is initialized

        TLogMessage queueItem;
        memcpy( queueItem.msgHeader, msgHeader, 24 );
        queueItem.logMsg = logMessage;

        m_logQueue.push_back( queueItem );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLogSvcClient::OnTaskCycleLogFlush( void )
{GUCEF_TRACE;

    if ( m_connectionInitialized )
    {
        char msg[ 6 ];
        msg[ 0 ] = LOGSVCMSGTYPE_DELIMITER;   // set delimiter for message: 1 byte
        CORE::UInt32 size = 1;
        memcpy( msg+1, &size, 4 );            // set the total message length : 4 bytes
        msg[ 5 ] = LOGSVCMSGTYPE_FLUSHLOG;    // set message type for message: 1 byte
        m_tcpClient.Send( &msg, 6 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcClient::SetApplicationName( const CORE::CString& applicationName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_appName = applicationName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLogSvcClient::GetApplicationName( void ) const
{GUCEF_TRACE;

    return m_appName;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcClient::SendAllQueuedItems( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    
    // We will copy the queue because the actual sending of the log
    // messages can cause more items to be queued
    TLogMsgStack logQueueCopy = m_logQueue;

    // Go through queue and send all items
    TLogMsgStack::reverse_iterator i = logQueueCopy.rbegin();
    while ( i != logQueueCopy.rend() )
    {
        TLogMessage& logMessage = (*i);

        // Send the logging msg header
        if ( m_tcpClient.Send( logMessage.msgHeader, 15 ) )
        {
            // Now send the logging text
            m_tcpClient.Send( logMessage.logMsg.C_String() ,
                              logMessage.logMsg.Length()   );
        }
        ++i;
    }

    // Clear the queue.
    // Any log messages that where added while sending will be dropped
    m_logQueue.clear();
}

/*-------------------------------------------------------------------------*/

void
CLogSvcClient::OnPumpedNotify( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventid  ,
                               CORE::CICloneable* eventdata )
{GUCEF_TRACE;


    if ( &m_tcpClient == notifier )
    {
        if ( COMCORE::CTCPClientSocket::ConnectedEvent == eventid )
        {
            // Now that we connected we should send the inital info
            // introducing who we are to the logging service
            CClientInitMessage initMessage;
            initMessage.SetProcessName( "Unknown" ); // @TODO
            initMessage.SetProcessId( "Unknown" ); // @TODO
            initMessage.SetApplicationName( m_appName );
            initMessage.SetClientVersion( ClientVersion );

            CORE::CDynamicBuffer buffer( initMessage.GetStreamedSize(), true );
            if ( initMessage.WriteToBuffer( buffer ) )
            {
                // Now actually send the message
                m_tcpClient.Send( buffer.GetBufferPtr(), buffer.GetDataSize() );
            }
        }
        else
        if ( COMCORE::CTCPClientSocket::DisconnectedEvent == eventid )
        {
            m_connectionInitialized = false;
        }
        else
        if ( COMCORE::CTCPClientSocket::DataRecievedEvent == eventid )
        {
            // Get access to the data from the event
            COMCORE::CTCPClientSocket::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
            const CORE::CDynamicBuffer& buffer = eData->GetData();

            // The only reponse we expect back from the server is a 1 byte status code
            // thus we just check 1 byte for that code
            CORE::UInt8 code = buffer.AsConstType< CORE::UInt8 >( 0 );
            if ( LOGSVCMSGTYPE_INITIALIZED == code )
            {
                SendAllQueuedItems();
                NotifyObservers( ConnectionInitializedEvent );
                m_connectionInitialized = true;
            }
            else
            if ( LOGSVCMSGTYPE_INCOMPATIBLE == code )
            {
                NotifyObservers( IncompatibleWithServerEvent );
                m_tcpClient.Close();
            }
        }
        else
        if ( COMCORE::CTCPClientSocket::ConnectingEvent == eventid )
        {
            NotifyObservers( ConnectingEvent );
        }
        else
        if ( COMCORE::CTCPClientSocket::SocketErrorEvent == eventid )
        {
            NotifyObservers( SocketErrorEvent );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
