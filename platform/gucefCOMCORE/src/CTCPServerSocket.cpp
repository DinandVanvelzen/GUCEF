/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include <string.h>                     /* standard string utils */
#include <malloc.h>                     /* Memory management */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#include "dvstrutils.h"                 /* my own string utils */
#include "tsprinting.h"			/* threadsafe printing */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"              /* we want the log manager for its defines */
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#include "CTCPServerSocket.h"           /* this file's header */

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

const CORE::CEvent CTCPServerSocket::ClientConnectedEvent = "GUCEF::COMCORE::CTCPServerSocket::ClientConnectedEvent";
const CORE::CEvent CTCPServerSocket::ClientDisconnectedEvent = "GUCEF::COMCORE::CTCPServerSocket::ClientDisconnectedEvent";
const CORE::CEvent CTCPServerSocket::ClientErrorEvent = "GUCEF::COMCORE::CTCPServerSocket::ClientErrorEvent";
const CORE::CEvent CTCPServerSocket::ServerSocketOpenedEvent = "GUCEF::COMCORE::CTCPServerSocket::ServerSocketOpenedEvent";
const CORE::CEvent CTCPServerSocket::ServerSocketClosedEvent = "GUCEF::COMCORE::CTCPServerSocket::ServerSocketClosedEvent";
const CORE::CEvent CTCPServerSocket::ServerSocketErrorEvent = "GUCEF::COMCORE::CTCPServerSocket::ServerSocketErrorEvent";
const CORE::CEvent CTCPServerSocket::ServerSocketClientErrorEvent = "GUCEF::COMCORE::CTCPServerSocket::ServerSocketClientErrorEvent";
const CORE::CEvent CTCPServerSocket::ServerSocketMaxConnectionsChangedEvent = "GUCEF::COMCORE::CTCPServerSocket::ServerSocketMaxConnectionsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct STCPServerConData
{
    SOCKET sockid;
    CORE::CString clientip;
    struct sockaddr_in clientaddr;
    struct timeval timeout;         /* timeout for blocking operations */
};
typedef struct STCPServerConData TTCPServerConData;

/*-------------------------------------------------------------------------*/

struct STCPServerSockData
{
    SOCKET sockid;           /* Winsock socket number */
    UInt32 maxcon;           /* maximum number of connections for this server socket */
    bool blocking;           /* is this a blocking or non-blocking server ? */
    SOCKADDR_IN serverinfo;  /* winsock info on the listening socket */
    struct timeval timeout;  /* timeout for blocking operations */
};
typedef struct STCPServerSockData TTCPServerSockData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CTCPServerSocket::CTCPServerSocket( const CORE::PulseGeneratorPtr& pulseGenerator ,
                                    bool blocking                                 ,
                                    UInt32 maxConnections                         )
    : CSocket()                               
    , _connections( maxConnections )
    , m_activeConnections() 
    , m_inactiveConnections()
    , _active( false )                        
    , _blocking( blocking )                   
    , m_port( 0 )                             
    , _datalock()
    , _timeout( 0 )
    , m_pulseGenerator( pulseGenerator )
    , m_maxUpdatesPerCycle( 10 )
    , m_autoReopenOnError( false )
    , m_lastListenFailed( false )
    , m_disconnectClientsIfIdle( false )
    , m_maxClientConnectionIdleTime( 600000 )
{GUCEF_TRACE;

    _data = GUCEF_NEW TTCPServerSockData;
    _data->blocking = blocking;
    _data->sockid = 0;
    _data->maxcon = maxConnections;

    _connections.reserve( maxConnections );

    TEventCallback callback( this, &CTCPServerSocket::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 callback                            );
}

/*-------------------------------------------------------------------------*/

CTCPServerSocket::CTCPServerSocket( bool blocking         ,
                                    UInt32 maxConnections )
    : CSocket()                               
    , _connections( maxConnections ) 
    , m_activeConnections() 
    , m_inactiveConnections()
    , _active( false )                        
    , _blocking( blocking )                   
    , m_port( 0 )      
    , _datalock()                       
    , _timeout( 0 )
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_maxUpdatesPerCycle( 10 )
    , m_autoReopenOnError( false )
    , m_lastListenFailed( false )
    , m_disconnectClientsIfIdle( false )
    , m_maxClientConnectionIdleTime( 600000 )
{GUCEF_TRACE;

    _data = GUCEF_NEW TTCPServerSockData;
    _data->blocking = blocking;
    _data->sockid = 0;
    _data->maxcon = maxConnections;

    _connections.reserve( maxConnections );

    TEventCallback callback( this, &CTCPServerSocket::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 callback                            );
}

/*-------------------------------------------------------------------------*/

CTCPServerSocket::~CTCPServerSocket()
{GUCEF_TRACE;

    /*
     *      Clean everything up
     */
    Close();

    /*
     *      Cleanup connection data
     */
    for ( UInt32 i=0; i<_connections.size(); ++i )
    {
        GUCEF_DELETE _connections[ i ];
    }
    _connections.clear();
    m_inactiveConnections.clear();
    m_activeConnections.clear();

    GUCEF_DELETE _data;
    _data = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::RegisterEvents( void )
{GUCEF_TRACE;

    ClientConnectedEvent.Initialize();
    ClientDisconnectedEvent.Initialize();
    ClientErrorEvent.Initialize();
    ServerSocketOpenedEvent.Initialize();
    ServerSocketClosedEvent.Initialize();
    ServerSocketErrorEvent.Initialize();
    ServerSocketClientErrorEvent.Initialize();
    ServerSocketMaxConnectionsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTCPServerConnection*
CTCPServerSocket::GetConnectionBySocketId( UInt32 socketId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<_connections.size(); ++i )
    {
        if ( socketId == _connections[ i ]->GetSocketID() )
        {
            return _connections[ i ];
        }
    }
	return NULL;
}

/*-------------------------------------------------------------------------*/

CTCPServerConnection*
CTCPServerSocket::GetConnection( UInt32 index )
{GUCEF_TRACE;

	return _connections[ index ];
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::OnPulse( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !_active && m_autoReopenOnError && m_lastListenFailed )
    {
        Listen();
    }     
    
    if ( _active )
    {
        /*
         *      Accept new incoming connections
         */
        AcceptClients();

        /*
         *      Update the client connections
         */
        if ( !_blocking )
        {
            TConnectionSet::iterator i = m_activeConnections.begin();
            while ( i != m_activeConnections.end() )
            {
                (*i)->Update( m_maxUpdatesPerCycle );
                ++i;
            }
            
            // If the update caused connnections to become (in)active in some way we deal with those here
            while ( !m_almostInactiveConnections.empty() )
            {
                CTCPServerConnection* connection = m_almostInactiveConnections.front();
                m_activeConnections.erase( connection );
                m_inactiveConnections.insert( connection );
                m_almostInactiveConnections.pop();
            }
            while ( !m_almostActiveConnections.empty() )
            {
                CTCPServerConnection* connection = m_almostActiveConnections.front();
                m_inactiveConnections.erase( connection );
                m_activeConnections.insert( connection );
                m_almostActiveConnections.pop();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerSocket::SetDisconnectClientConnectionIfIdle( bool disconnectIfIdle )
{GUCEF_TRACE;

    m_disconnectClientsIfIdle = disconnectIfIdle;

    TConnectionSet::iterator i = m_activeConnections.begin();
    while ( i != m_activeConnections.end() )
    {
        (*i)->SetDisconnectIfIdle( disconnectIfIdle );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CTCPServerSocket::GetDisconnectClientConnectionIfIdle( void ) const
{GUCEF_TRACE;

    return m_disconnectClientsIfIdle;
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerSocket::SetClientConnectionMaxIdleDurationInMs( UInt32 maxIdleTimeInMs )
{GUCEF_TRACE;

    m_maxClientConnectionIdleTime = maxIdleTimeInMs;

    TConnectionSet::iterator i = m_activeConnections.begin();
    while ( i != m_activeConnections.end() )
    {
        (*i)->SetMaxIdleDurationInMs( maxIdleTimeInMs );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32 
CTCPServerSocket::GeClientConnectiontMaxIdleDurationInMs( void ) const 
{GUCEF_TRACE;

    return m_maxClientConnectionIdleTime;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::SetAutoReOpenOnError( bool autoReOpen )
{GUCEF_TRACE;

    m_autoReopenOnError = autoReOpen;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerSocket::GetAutoReOpenOnError( void ) const
{GUCEF_TRACE;

    return m_autoReopenOnError;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::SetMaxUpdatesPerCycle( UInt32 maxUpdates )
{GUCEF_TRACE;

    m_maxUpdatesPerCycle = maxUpdates;
}

/*-------------------------------------------------------------------------*/


UInt32
CTCPServerSocket::GetMaxUpdatesPerCycle( void ) const
{GUCEF_TRACE;

    return m_maxUpdatesPerCycle;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::AcceptClients( void )
{GUCEF_TRACE;

    int s = 0;           /* s is where the data is stored from the select function */
    int nfds = 0;        /* This is used for Compatibility */
    
    fd_set conn;         /* Setup the read variable for the Select function */
    fd_set exceptfds;    /* Setup the except variable for the select function */

    FD_ZERO( &conn );       // Set the data in conn to nothing
    FD_ZERO( &exceptfds );

    FD_SET( _data->sockid, &conn );   // Tell it to get the data from the Listening Socket
    FD_SET( _data->sockid, &exceptfds );

    if ( !_data->blocking )
    {
        /*
         *  Because we are in non-blocking mode we must make sure the timeout
         *  structure is zero'd or it will block
         */
        memset( &_data->timeout, 0, sizeof( struct timeval ) );

        /*
         *      Up the nfds value by one, shouldnt be the same for each
         *      client that connects for compatibility reasons
         *      Doing a select first will ensure that we don't block on accept()
         */
        nfds = (int)_data->sockid+1;
        s = select( nfds            ,
                    &conn           ,
                    GUCEF_NULL      ,
                    &exceptfds      ,
                    &_data->timeout ); // Is there any data coming in?

        if ( s > 0 ) /* Someone is trying to Connect */
        {
            if ( (UInt32)m_activeConnections.size() >= _data->maxcon )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TCPServerSocket: More connections incoming than the allowed max, ignoring them. Max: " + CORE::UInt32ToString( _data->maxcon ) );
                return;
            }
                
            TConnectionSet::iterator i = m_inactiveConnections.begin();
            if ( i != m_inactiveConnections.end() )
            {
                CTCPServerConnection* clientcon = (*i);
                if ( !clientcon->IsActive() )
                {
                    int error = 0;
                    int aint = sizeof( struct sockaddr );
                    clientcon->_data->sockid = dvsocket_accept( _data->sockid                                    ,
                                                                (struct sockaddr*) &clientcon->_data->clientaddr ,
                                                                &aint                                            ,
                                                                &error                                           );

                    /*
                     *      Client Connected
                     *      Store the Ip of the Client that Just Connected.
                     */
                    char clientip[ 20 ];
                    dvsocket_inet_ntoa( clientcon->_data->clientaddr.sin_addr ,
                                        clientip                              );
                    clientcon->_data->clientip = clientip;

                    /*
                     *      Set the socket into the desired mode of operation
                     *      ie blocking or non-blocking mode
                     */
                    if ( !SetBlockingMode( _data->sockid ,
                                            _blocking     ) )
                    {
                        clientcon->_data->sockid = 0;
                        return;
                    }
                    clientcon->_blocking = _blocking;
                    if ( !_blocking )
                    {
                        /*
                         *  Because we are in non-blocking mode we must make sure the timeout
                         *  structure is zero'd or it will block
                         */
                        memset( &clientcon->_data->timeout, 0, sizeof( struct timeval ) );
                    }

                    clientcon->_active = true;
                    clientcon->SetMaxIdleDurationInMs( m_maxClientConnectionIdleTime );
                    clientcon->SetDisconnectIfIdle( m_disconnectClientsIfIdle );
                    m_almostActiveConnections.push( clientcon );

                    /*
                     *      Call the on client connect event handler
                     */
                    struct SConnectionInfo eData;
                    eData.hostAddress = clientcon->GetRemoteHostAddress();
                    eData.connection = clientcon;
                    eData.connectionIndex = clientcon->GetConnectionIndex();

                    TClientConnectedEventData cloneableEventData( eData );
                    NotifyObservers( ClientConnectedEvent, &cloneableEventData );

                    return;
                }
                ++i;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerSocket::IsActive( void ) const
{GUCEF_TRACE;

    return _active;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerSocket::IsBlocking( void ) const
{GUCEF_TRACE;

    return _blocking;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerSocket::Listen( void )
{GUCEF_TRACE;

    return ListenOnPort( m_port );
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerSocket::ListenOnPort( UInt16 servport )
{GUCEF_TRACE;

    /*
     *      Close all existing connections
     */
    if ( IsActive() )
    {
            Close();
    }
    m_port = servport;

    // Create connection slot objects for any slots not yet initialized.
    // The pre-sized _connections vector starts with all null pointers; we allocate
    // only slots that haven't been created yet so that re-entrant ListenOnPort calls
    // (e.g. after Close()) don't leak the previously created connection objects.
    for ( UInt32 i=0; i<_data->maxcon; ++i )
    {
        if ( GUCEF_NULL == _connections[ i ] )
        {
            _connections[ i ] = GUCEF_NEW CTCPServerConnection( this, i );
            m_inactiveConnections.insert( _connections[ i ] );
        }
    }

    int error = 0;
    _data->sockid = dvsocket_socket( AF_INET     ,    /* Go over TCP/IP */
                                     SOCK_STREAM ,    /* This is a stream-oriented socket */
                                     IPPROTO_TCP ,    /* Use TCP rather than UDP */
                                     &error      );

    if ( _data->sockid == INVALID_SOCKET )
    {
        m_lastListenFailed = true;
        if ( m_autoReopenOnError ) 
            m_pulseGenerator->RequestPeriodicPulses( this, 10 );

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CTCPServerSocket: Socket error: " + CORE::Int32ToString( error ) );
        TServerSocketErrorEventData eData( error );
        NotifyObservers( ServerSocketErrorEvent, &eData );
        return false;
    }

    /* Set the desired blocking mode */
    if ( !SetBlockingMode( _data->sockid ,
                           _blocking     ) )
    {
        m_lastListenFailed = true;
        if ( m_autoReopenOnError ) 
            m_pulseGenerator->RequestPeriodicPulses( this, 10 );

        NotifyObservers( ServerSocketErrorEvent );
        return false;
    }

	/*
	 *      We set the protocol family to TCP/IP
	 */
	_data->serverinfo.sin_family = AF_INET;

	/*
	 *      Since this socket is listening for connections, any local address will do
	 */
	_data->serverinfo.sin_addr.s_addr = INADDR_ANY;

    /*
     *      Convert integer servport to network-byte order
     *      and insert into the port field
     */
	_data->serverinfo.sin_port = htons( servport );

    /*
     *      Bind the socket to our local server address
     */
	int retval = dvsocket_bind( _data->sockid                  ,
	                            (LPSOCKADDR)&_data->serverinfo ,
	                            sizeof( _data->serverinfo )    ,
	                            &error                         );

	if ( retval == SOCKET_ERROR )
	{
        m_lastListenFailed = true;
        if ( m_autoReopenOnError ) 
            m_pulseGenerator->RequestPeriodicPulses( this, 10 );

        GUCEF_ERROR_LOG( 1, "CTCPServerSocket: Socket error: " + CORE::Int32ToString( error ) );
	    TServerSocketErrorEventData eData( error );
	    NotifyObservers( ServerSocketErrorEvent, &eData );
	    return false;
	}

    // Before we actually open the socket lets give observers the chance to pre-allocate things if needed
    // based on the max number of connections.
    TServerSocketMaxConnectionsChangedEventData maxConEData( _data->maxcon );
    NotifyObservers( ServerSocketMaxConnectionsChangedEvent, &maxConEData );

    /*
     *      Make the socket listen
     *
     *      Up to maxcon connections may wait at any
     *      one time to be accept()'ed
     */
	retval = dvsocket_listen( _data->sockid      ,
	                          _data->maxcon      ,
	                          &error             );

	if ( retval == SOCKET_ERROR )
	{
		m_lastListenFailed = true;
        if ( m_autoReopenOnError ) 
            m_pulseGenerator->RequestPeriodicPulses( this, 10 );

        TServerSocketErrorEventData eData( error );
		NotifyObservers( ServerSocketErrorEvent, &eData );
		return false;
	}
	_active = true;
    m_lastListenFailed = false;
    NotifyObservers( ServerSocketOpenedEvent );

	/*
	 *      Accept new connections if there are any.
	 */
    AcceptClients();

    m_pulseGenerator->RequestPeriodicPulses( this, 10 );

    return true;
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerSocket::SetMaxConnections( UInt32 maxConnections, bool dropConnections )
{GUCEF_TRACE;
    
    UInt32 oldMax = _data->maxcon;
    _data->maxcon = maxConnections;

    if ( oldMax > maxConnections )
    {
        Lock();
        if ( dropConnections )
        {
            Int32 nrToDrop = (Int32) m_activeConnections.size() - (Int32) maxConnections;
            for ( Int32 i=0; i<nrToDrop; ++i )
            {
                CTCPServerConnection* connection = *(m_activeConnections.begin());
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TCPServerSocket: Closing connection " + CORE::UInt32ToString( connection->GetSocketID() ) + " due to lowered max allowed connection");
                connection->Close();
            }
        }
        Unlock();
    }

    TServerSocketMaxConnectionsChangedEventData maxConEData( _data->maxcon );
    NotifyObservers( ServerSocketMaxConnectionsChangedEvent, &maxConEData );
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerSocket::GetMaxConnections( void ) const
{GUCEF_TRACE;

    return (UInt32) _data->maxcon;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerSocket::GetActiveCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_activeConnections.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerSocket::GetBytesReceived( bool resetCounter )
{GUCEF_TRACE;

    UInt32 bytesReceived = 0;
    _datalock.Lock();

    // As a performance compromise we only retrieve and sum for active connections
    // this has the downside that if a connection closed between counter polling cycles you would miss
    // whatever the extra count would have been from that connection
    TConnectionSet::iterator i = m_activeConnections.begin();
    while ( i != m_activeConnections.end() )
    {
        bytesReceived += (*i)->GetBytesReceived( resetCounter );
        ++i;
    }
    
    _datalock.Unlock();
    return bytesReceived;
}

/*-------------------------------------------------------------------------*/

UInt32 
CTCPServerSocket::GetBytesTransmitted( bool resetCounter )
{GUCEF_TRACE;

    UInt32 bytesTransmitted = 0;
    _datalock.Lock();

    // As a performance compromise we only retrieve and sum for active connections
    // this has the downside that if a connection closed between counter polling cycles you would miss
    // whatever the extra count would have been from that connection
    TConnectionSet::iterator i = m_activeConnections.begin();
    while ( i != m_activeConnections.end() )
    {
        bytesTransmitted += (*i)->GetBytesTransmitted( resetCounter );
        ++i;
    }
    
    _datalock.Unlock();
    return bytesTransmitted;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::Close( void )
{GUCEF_TRACE;

    /*
     *      Close the socket
     *      By setting active to false the thread will terminate
     */
    _datalock.Lock();

    if ( IsActive() )
    {
        int errorCode = 0;
        dvsocket_closesocket( _data->sockid, &errorCode );

        NotifyObservers( ServerSocketClosedEvent );

        TConnectionSet::iterator i = m_activeConnections.begin();
        while ( i != m_activeConnections.end() )
        {
            (*i)->CloseImp( false, true, false, true );
            m_inactiveConnections.insert( (*i) );
            ++i;
        }
        m_activeConnections.clear();
        _active = false;
    }
    _datalock.Unlock();

    m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
}

/*-------------------------------------------------------------------------*/

bool 
CTCPServerSocket::CloseClientConnection( UInt32 connectionIndex )
{GUCEF_TRACE;

    Lock();
    if ( connectionIndex < _connections.size() )
    {
        _connections[ connectionIndex ]->Close();
        Unlock();
        return true;
    }
    Unlock();
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CTCPServerSocket::SetPort( UInt16 port )
{GUCEF_TRACE;

    bool wasActive = false;
    if ( IsActive() )
    {
        wasActive = true;
        Close();
    }

    m_port = port;

    if ( wasActive )
    {
        return Listen();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt16
CTCPServerSocket::GetPort( void ) const
{GUCEF_TRACE;

    return m_port;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::OnClientConnectionClosed( CTCPServerConnection* connection ,
                                            const UInt32 connectionid        ,
                                            bool closedByClient              ,
                                            bool updateActiveLists           )
{GUCEF_TRACE;

    TClientDisconnectedEventData cloneableEventData;    
    struct SDisconnectInfo& eData = cloneableEventData.GetData();
    eData.connectionInfo.hostAddress = connection->GetRemoteHostAddress();
    eData.connectionInfo.connection = connection;
    eData.connectionInfo.connectionIndex = connectionid;
    eData.closedByClient = closedByClient;

    if ( updateActiveLists )
    {
        m_almostInactiveConnections.push( connection );
    }
    NotifyObservers( ClientDisconnectedEvent, &cloneableEventData );
}

/*-------------------------------------------------------------------------*/

void
CTCPServerSocket::GetListenAddress( CHostAddress& listenAddress ) const
{GUCEF_TRACE;

    _datalock.Lock();
    listenAddress.SetHostnameAndPort( "localhost", m_port );
    _datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerSocket::SendToAllClients( const void* dataSource , 
                                    const UInt32 dataSize  )
{GUCEF_TRACE;

    bool totalSuccess = true;
    _datalock.Lock();
    if ( IsActive() )
    {
        TConnectionSet::iterator i = m_activeConnections.begin();
        while ( i != m_activeConnections.end() )
        {
            totalSuccess = totalSuccess && (*i)->Send( dataSource, dataSize );
            ++i;
        }
    }
    _datalock.Unlock();
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CTCPServerSocket::SendToConnection( UInt32 connectionIndex                ,
                                    const void* dataSource                , 
                                    const UInt32 dataSize                 ,
                                    const CIPv4Address* remoteClientIPCheck )
{GUCEF_TRACE;

    MT::CObjectScopeLock serverLock( AsLockable() );
    
    if ( connectionIndex < _connections.size() )
    {
        CTCPServerConnection* connection = _connections[ connectionIndex ];
        
        serverLock.EarlyUnlock();
        MT::CObjectScopeLock connectionLock( connection->AsLockable() );

        if ( connection->IsActive() )
        {
            if ( GUCEF_NULL != remoteClientIPCheck )
            {
                if ( connection->GetRemoteIP() != *remoteClientIPCheck )
                {
                    // Refusing to send because the IP check failed.
                    return false;
                }

                return connection->Send( dataSource, dataSize );
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTCPServerSocket::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return _datalock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTCPServerSocket::Unlock( void ) const
{GUCEF_TRACE;

    return _datalock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
