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

//-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------//

#include <assert.h>
#include <string.h>

#ifndef DVSTRUTILS_H
#include "dvstrutils.h"
#define DVSTRUTILS_H
#endif /* DVSTRUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H ? */

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#include "CTCPClientSocket.h"           /* Header with class for these utils */

#ifdef ACTIVATE_MEMORY_MANAGER
  #ifndef GUCEF_NEW_ON_H
  #include "gucef_new_on.h"   /* Use the GUCEF memory manager instead of the standard manager ? */
  #define GUCEF_NEW_ON_H
  #endif /* GUCEF_NEW_ON_H ? */
#endif /* ACTIVATE_MEMORY_MANAGER ? */

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

const CORE::CEvent CTCPClientSocket::ConnectingEvent = "GUCEF::COMCORE::CTCPClientSocket::ConnectingEvent";

//-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------//

#define MAX_PULSE_INTERVAL_IN_MS    10
#define BUFFER_READ_SIZE            16

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      struct with os implementation specific data
 */
struct CTCPClientSocket::STCPClientSockData
{
    SOCKET sockid;
    SOCKADDR_IN serverinfo;
    struct timeval timeout;         /* timeout for blocking operations */
};
typedef struct CTCPClientSocket::STCPClientSockData TTCPClientSockData;

//-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------//

CTCPClientSocket::CTCPClientSocket( CORE::PulseGeneratorPtr pulseGenerator ,
                                    bool blocking                          )
    : CTCPConnection()
    , _blocking( blocking )
    , _active( false )
    , datalock()
    , m_readbuffer()
    , m_sendBuffer()
    , m_sendOpBuffer()
    , m_maxreadbytes( 0 )
    , m_hostAddress()
    , m_ipv4Target()
    , m_isConnecting( false )
    , m_pulseGenerator( pulseGenerator )
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    , m_coaleseDataSends( true )
    #else
    , m_coaleseDataSends( false )
    #endif
    , m_maxUpdatesPerCycle( 10 )
    , m_autoReconnectOnError( false )
    , m_lastConnFailed( false )
    , m_useTcpKeepAlive( false )
    , m_maxKeepAliveProbeFails( 3 )
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    _data = GUCEF_NEW TTCPClientSockData;
    assert( _data != NULL );
    memset( &_data->timeout, 0, sizeof( struct timeval ) );

    TEventCallback callback( this, &CTCPClientSocket::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 callback                            );
}

/*-------------------------------------------------------------------------*/

CTCPClientSocket::CTCPClientSocket( bool blocking )
    : CTCPConnection()
    , _blocking( blocking )
    , _active( false )
    , datalock()
    , m_readbuffer()
    , m_sendBuffer()
    , m_sendOpBuffer()
    , m_maxreadbytes( 0 )
    , m_hostAddress()
    , m_isConnecting( false )
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    , m_coaleseDataSends( true )
    #else
    , m_coaleseDataSends( false )
    #endif
    , m_maxUpdatesPerCycle( 10 )
    , m_autoReconnectOnError( false )
    , m_lastConnFailed( false )
    , m_useTcpKeepAlive( false )
    , m_maxKeepAliveProbeFails( 3 )
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    _data = GUCEF_NEW TTCPClientSockData;
    assert( _data != NULL );
    memset( &_data->timeout, 0, sizeof( struct timeval ) );

    TEventCallback callback( this, &CTCPClientSocket::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 callback                            );
}

/*-------------------------------------------------------------------------*/

CTCPClientSocket::~CTCPClientSocket()
{GUCEF_TRACE;

    Close();
    GUCEF_DELETE _data;
    _data = NULL;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::IsActive( void ) const
{GUCEF_TRACE;

        return _active;
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectingEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::IsBlocking( void ) const
{GUCEF_TRACE;

    return _blocking;
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::SetMaxRead( UInt32 mr )
{GUCEF_TRACE;

    Lock();
    m_maxreadbytes = mr;
    Unlock();
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPClientSocket::GetMaxRead( void ) const
{GUCEF_TRACE;
        return m_maxreadbytes;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::Reconnect( bool blocking )
{GUCEF_TRACE;

    Lock();
    bool retval = ConnectTo( m_hostAddress ,
                             blocking      );
    Unlock();
    return retval;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::ConnectTo( const CORE::CString& remoteaddr ,
                             UInt16 port                     ,
                             bool blocking                   )
{GUCEF_TRACE;

    if ( remoteaddr.Length() == 0 ) return false;

    // Don't bother trying to connect if the DNS resolution fails
    CHostAddress remoteAddress;

    if ( !remoteAddress.SetHostnameAndPort( remoteaddr, port ) )
        return false;

    CloseImp();

    Lock();
    m_hostAddress = remoteAddress;
    bool success = Connect( blocking );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::ConnectTo( const CIPv4Address& address ,
                             bool blocking             )
{GUCEF_TRACE;

    CloseImp();

    Lock();
    m_hostAddress = address;
    bool success = Connect( blocking );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::ConnectTo( const CHostAddress& address ,
                             bool blocking               )
{GUCEF_TRACE;

    CloseImp();

    Lock();
    m_hostAddress = address;
    bool success = Connect( blocking );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::Connect( bool blocking )
{GUCEF_TRACE;

    _active = true;

    int errorCode = 0;
    _data->sockid = dvsocket_socket( AF_INET     ,  /* Go over TCP/IP */
                                     SOCK_STREAM ,  /* This is a stream-oriented socket */
                                     IPPROTO_TCP ,  /* Use TCP rather than UDP */
                                     &errorCode  );

    // Check for an error
    if ( errorCode != 0 )
    {
        // After a socket error you must always close the connection.
        _active = false;
        m_lastConnFailed = true;

        // Notify our users of the error
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorCode ) );

        Unlock();

        TSocketErrorEventData eData( errorCode );
        if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return false;
        return false;
    }

    if ( _data->sockid == INVALID_SOCKET )
    {
        Unlock();
        return false;
    }

    /* Set the desired blocking mode */
    if ( !SetBlockingMode( _data->sockid ,
                           _blocking     ) )
    {
        _active = false;
        Unlock();
        return false;
    }

    /* we want to use the internet protocol */
    _data->serverinfo.sin_family = AF_INET;

	/*
	 *  At this point, we've successfully retrieved vital information about the server,
	 *  including its hostname, aliases, and IP addresses.
	 */
    m_ipv4Target = m_hostAddress.GetRandomIPv4Address();
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Using IP " + m_ipv4Target.AddressAsString() + " for target " + m_hostAddress.GetHostname() );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _data->serverinfo.sin_addr.S_un.S_addr = m_ipv4Target.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    _data->serverinfo.sin_addr.s_addr = m_ipv4Target.GetAddress();
    #endif

    /*
     *  Change port number to network-byte order and
     *  then insert into the server port info field
     */
	_data->serverinfo.sin_port = m_ipv4Target.GetPort();

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    int noDelayFlag = (m_coaleseDataSends ? 1 : 0);
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_TCP, TCP_NODELAY, (char*) &noDelayFlag, sizeof(noDelayFlag), &errorCode ) )
    {
        _active = m_lastConnFailed = false;

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::ToString( this ) + "): Failed to set no delay mode \"" + CORE::BoolToString( m_coaleseDataSends )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
        Unlock();
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::ToString( this ) + "): Successfully set no delay mode \"" + CORE::BoolToString( m_coaleseDataSends ) + "\" on socket" );

    #else
        // TCP_NODELAY is not supported in Emscripten/WebAssembly
        // WebSockets don't have the concept of Nagle's algorithm
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::ToString( this ) + "): TCP_NODELAY not supported in WebAssembly environment, skipping" );
    #endif

    int allowAddressReuse = 1;
    if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_REUSEADDR, (const char*) &allowAddressReuse, sizeof(int), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Failed to set address reuse mode \"" + CORE::BoolToString( allowAddressReuse != 0 )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    else
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Successfully set address reuse mode \"" + CORE::BoolToString( allowAddressReuse != 0 ) + "\" on socket" );

    #ifdef SO_REUSEPORT
    int allowPortReuse = 1;
    if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_REUSEPORT, (const char*) &allowPortReuse, sizeof(int), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Failed to set port reuse mode \"" + CORE::BoolToString( allowPortReuse != 0 )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    else
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Successfully set port reuse mode \"" + CORE::BoolToString( allowPortReuse != 0 ) + "\" on socket" );
    #endif

    int keepAlive = (m_useTcpKeepAlive ? 1 : 0);
    if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_KEEPALIVE, (const char*) &keepAlive, sizeof(int), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Failed to set keep alive mode \"" + CORE::BoolToString( m_useTcpKeepAlive )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    else
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Successfully set keep alive mode \"" + CORE::BoolToString( m_useTcpKeepAlive ) + "\" on socket" );

    //int maxFails = (int)m_maxKeepAliveProbeFails;
    //if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_TCP, TCP_KEEPCNT, (const char*) &maxFails, sizeof(int), &errorCode ) )
    //{
    //    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Failed to set keep alive max probe failures \"" + CORE::Int32ToString( maxFails )
    //        + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    //}
    //else
    //GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Successfully set keep alive max probe failures \"" + CORE::Int32ToString( maxFails ) + "\" on socket" );

	/*
	 *      We now attempt to create a connection.
	 */
    if ( dvsocket_connect( _data->sockid                  ,
                           (LPSOCKADDR)&_data->serverinfo ,
                           sizeof( struct sockaddr)       ,
                           &errorCode                     ) == SOCKET_ERROR )
    {
        // It is normal for DVSOCKET_EWOULDBLOCK to be reported as the result from calling
        // connect on a nonblocking SOCK_STREAM socket, since some time must elapse
        // for the connection to be established.
        if ( errorCode != DVSOCKET_EWOULDBLOCK )
        {
            // After a socket error you must always close the connection.
            _active = false;
            m_lastConnFailed = true;
            Unlock();

            // Check for an error
            if ( errorCode != 0 )
            {
                // Notify our users of the error
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorCode ) );
                TSocketErrorEventData eData( errorCode );
                if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return false;
            }
            return false;
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Connecting to " + m_hostAddress.HostnameAndPortAsString() );

            m_readbuffer.Clear();

            m_isConnecting = true;

            Unlock();

            NotifyObservers( ConnectingEvent );

            // Check if the caller wants this specific call to act as a blocking call but has the socket set as non-blocking
            if ( !_blocking && blocking )
            {
                while ( m_isConnecting && _active )
                {
                    // We will manually cycle here to wait for the call to complete
                    OnPulse( NULL                              ,
                             CORE::CPulseGenerator::PulseEvent ,
                             NULL                              );

                    if ( m_isConnecting && _active )
                        MT::PrecisionDelay( 25 );
                }

                m_pulseGenerator->RequestPeriodicPulses( this, MAX_PULSE_INTERVAL_IN_MS );
            }
            else
            {
                m_pulseGenerator->RequestPeriodicPulses( this, MAX_PULSE_INTERVAL_IN_MS );
            }

            return true;
        }
    }
    return false;
}


/*-------------------------------------------------------------------------*/

const CORE::CString&
CTCPClientSocket::GetRemoteHostName( void ) const
{GUCEF_TRACE;

    return m_hostAddress.GetHostname();
}

/*-------------------------------------------------------------------------*/

UInt16
CTCPClientSocket::GetRemoteTCPPort( void ) const
{GUCEF_TRACE;

    return m_hostAddress.GetPortInHostByteOrder();
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CTCPClientSocket::GetRemoteIP( void ) const
{GUCEF_TRACE;

    return m_ipv4Target;
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::CheckRecieveBuffer( void )
{GUCEF_TRACE;

    if ( !_blocking && _active )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Checking the recieve buffer for incoming data" );

        Lock();

        /*
         *      Since this is a non-blocking socket we need to poll for received data
         */
        int bytesrecv;
        int errorcode = 0;
        UInt32 readblocksize;
        m_maxreadbytes > 0 ? readblocksize = m_maxreadbytes : readblocksize = 1024;

        // reset our logical data size delimiter
        m_readbuffer.SetDataSize( 0 );

        do
        {
            // make sure the buffer can hold another read block
            m_readbuffer.SetBufferSize( m_readbuffer.GetDataSize()+readblocksize, false );

            // read an additional block
            bytesrecv = dvsocket_recv( _data->sockid                                                                ,
                                       static_cast<char*>(m_readbuffer.GetBufferPtr()) + m_readbuffer.GetDataSize() ,
                                       readblocksize                                                                ,
                                       0                                                                            ,
                                       &errorcode                                                                   );

            // Check for an error
            if ( ( bytesrecv == SOCKET_ERROR ) || ( errorcode != 0 ) )
            {
                m_lastConnFailed = true;
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorcode ) );

                // Notify our users of the error
                TSocketErrorEventData eData( errorcode );
                if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return;

                // After a socket error you must always close the connection.
                CloseImp();

                Unlock();
                return;
            }
            else
            if ( bytesrecv == 0 )
            {
                // For the purpose of reconnecting we consider the connection failed if is closed for any reason other than
                // purposefully closing it on the client's side as the reconnect setting indicates the intent of a persistent connection
                m_lastConnFailed = true;

                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): The server has closed the connection" );

                // The connection has been closed on us
                CloseImp();

                Unlock();
                return;
            }

            // Increase the logical data size delimiter by the amount we just copied into the buffer
            m_readbuffer.SetDataSize( m_readbuffer.GetDataSize() + bytesrecv );
            m_bytesReceived += bytesrecv;

            // Is there a limit on the number of bytes we want to read ?
            if ( m_maxreadbytes > 0 )
            {
                // Check to make sure we do not exceed the maximum
                if ( m_maxreadbytes <= m_readbuffer.GetDataSize() )
                {
                    // This is a busy socket, don't yield to the scheduler
                    m_pulseGenerator->RequestImmediatePulse();
                    break;
                }
            }
        }
        while ( bytesrecv == readblocksize );

        if ( m_readbuffer.GetDataSize() > 0 )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Received " + CORE::UInt32ToString( m_readbuffer.GetDataSize() ) + " bytes" );

            CORE::CDynamicBuffer linkedBuffer;
            linkedBuffer.LinkTo( m_readbuffer.GetConstBufferPtr(), m_readbuffer.GetDataSize() );
            TDataRecievedEventData cData( &linkedBuffer );
            if ( !NotifyObservers( DataRecievedEvent, &cData ) ) return;
        }

        Unlock();
    }
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::OnPulse( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !_active && m_autoReconnectOnError && m_lastConnFailed && !m_isConnecting )
    {
        Reconnect( false );
    }

    if ( !_blocking && _active )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Pulse received" );

        fd_set writefds;     /* Setup the write variable for the select function */
        fd_set readfds;      /* Setup the read variable for the select function */
        fd_set exceptfds;    /* Setup the except variable for the select function */

        FD_ZERO( &writefds );
        FD_ZERO( &readfds );
        FD_ZERO( &exceptfds );

        Lock();

        FD_SET( _data->sockid, &writefds );
        FD_SET( _data->sockid, &readfds );
        FD_SET( _data->sockid, &exceptfds );

        int errorCode = 0;
        int selectResult = dvsocket_select( (int)_data->sockid+1 ,
                                            &readfds             ,
                                            &writefds            ,
                                            &exceptfds           ,
                                            &_data->timeout      ,
                                            &errorCode           );

        if ( selectResult != SOCKET_ERROR )
        {
            // something happened on the socket

            if ( FD_ISSET( _data->sockid, &exceptfds ) )
            {
                // This set is watched for "exceptional conditions".
                // In practice, only one such exceptional condition is common: the availability of out-of-band (OOB) data for reading from a TCP socket.
                // (One other less common case where select() indicates an exceptional condition occurs with pseudoterminals in packet mode;
                // see tty_ioctl().) After select() has returned, exceptfds will be cleared of all file descriptors except for those for which an
                // exceptional condition has occurred.

                m_lastConnFailed = true;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): An exception flag was set on the socket file descriptor. Typically this signals out-of-band data" );
                Unlock();

                TSocketErrorEventData eData( errorCode );
                if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return;

                CloseImp();
                return;
            }

            if ( FD_ISSET( _data->sockid, &readfds ) )
            {
                /* data can be read from the socket */
                CheckRecieveBuffer();
            }
            else
            if ( m_isConnecting )
            {
                // Check if the socket is now ready for writing
                if ( FD_ISSET( _data->sockid, &writefds ) )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket is now connected to the server, TCP channel established" );

                    // We are now connected
                    m_isConnecting = false;
                    if ( !NotifyObservers( ConnectedEvent ) ) return;
                }
            }
        }
        else
        {
            m_lastConnFailed = true;
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket error occured (select call failed): " + CORE::Int32ToString( errorCode ) );
            Unlock();

           /* select call failed */
            TSocketErrorEventData eData( errorCode );
            if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return;

            CloseImp();
            return;
        }

        // Check if we still have data queued to be sent,..
        while ( m_sendBuffer.HasBufferedData() )
        {
            // read an entire block
            if ( 0 != m_sendBuffer.ReadBlockTo( m_sendOpBuffer ) )
            {
                // We will try looping until we have transmitted all the data
                int error;
                UInt32 totalBytesSent = 0;
                Int32 wbytes;
                while ( totalBytesSent < m_sendOpBuffer.GetDataSize() )
                {
                    // perform a send, trying to send as much of the given data as possible
                    const Int8* data = static_cast< const Int8* >( m_sendOpBuffer.GetConstBufferPtr() );
                    Int32 remnant = m_sendOpBuffer.GetDataSize() - totalBytesSent;
                    wbytes = dvsocket_send( _data->sockid       ,
                                            data+totalBytesSent ,
                                            remnant             ,
                                            0                   ,
                                            &error              );
                    if ( wbytes != SOCKET_ERROR )
                    {
                        // we where able to send at least some of the data
                        totalBytesSent += wbytes;
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Succeeded in delayed sending " + CORE::Int32ToString( wbytes ) + "bytes of queued data" );
                    }
                    else
                    {
                        // Socket error,..
                        // Check if we have to delay sending the data
                        if ( DVSOCKET_EWOULDBLOCK == error && !_blocking )
                        {
                            // Cannot send now,... try again next pulse
                            // We have to place remaining data we grabbed from the send buffer back in
                            // the send buffer in a FIFO manner
                            m_sendBuffer.WriteAtFrontOfQueue( data+totalBytesSent ,
                                                              remnant             ,
                                                              1                   );

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Unable to delayed send queued data at this time" );

                            Unlock();
                            return;
                        }
                        else
                        {
                            m_lastConnFailed = true;
                            Unlock();

                            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( error ) );

                            TSocketErrorEventData eData( error );
                            NotifyObservers( SocketErrorEvent, &eData );

                            CloseImp();
                            return;
                        }
                    }
                }
            }
        }

        Unlock();
    }
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::Close( void )
{GUCEF_TRACE;

    // This is an explicit close request so clear the last fail flag to prevent reconnects
    m_lastConnFailed = false;

    CloseImp();
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::CloseImp( void )
{GUCEF_TRACE;

    /*
     *      close the socket connection
     */
    Lock();
    if ( IsActive() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Closing active socket" );

        /* prevent any further sends on the socket */
        shutdown( _data->sockid ,
                  1             );

        /* close the socket, this de-allocates the winsock buffers for the socket */
        int errorCode;
        if ( 0 != dvsocket_closesocket( _data->sockid, &errorCode ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): closesocket() failed. ErrorCode: " + CORE::Int32ToString( errorCode ) );
        }
        _active = false;
        m_isConnecting = false;

        // Wipe the buffers
        m_sendBuffer.Clear( false );
        m_sendOpBuffer.Clear( false );
        m_readbuffer.Clear( false );

        if ( !m_lastConnFailed || ( !m_autoReconnectOnError && m_lastConnFailed ) )
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );

        Unlock();

        if ( !NotifyObservers( DisconnectedEvent ) ) return;
    }
    else
        Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::Send( const void* dataSource ,
                        const UInt32 dataSize  )
{GUCEF_TRACE;

    return Send( dataSource ,
                 dataSize   ,
                 0          );
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPClientSocket::GetBufferedDataToSendInBytes( void ) const
{GUCEF_TRACE;

    Lock();
    UInt32 bBytes = m_sendBuffer.GetBufferedDataSizeInBytes();
    Unlock();
    return bBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPClientSocket::GetBytesReceived( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 bytesReceived = m_bytesReceived;
        m_bytesReceived = 0;
        return bytesReceived;
    }
    else
        return m_bytesReceived;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPClientSocket::GetBytesTransmitted( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 bytesTransmitted = m_bytesTransmitted;
        m_bytesTransmitted = 0;
        return bytesTransmitted;
    }
    else
        return m_bytesTransmitted;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::Send( const void* data ,
                        UInt32 length    ,
                        UInt32 timeout   )
{GUCEF_TRACE;

    if ( IsActive() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Sending data of length " + CORE::UInt32ToString( length ) + " with timeout " + CORE::UInt32ToString( timeout ) );

        Lock();

        // notify observers that we are sending data
        CORE::CDynamicBuffer linkedBuffer;
        linkedBuffer.LinkTo( data, length );
        TDataRecievedEventData cData( &linkedBuffer );
        if ( !NotifyObservers( DataSentEvent, &cData ) ) return false;

        // Check if we still have data queued to be sent,..
        // TCP has to be in-order so we will have to queue the new data behind the already queued data
        if ( m_sendBuffer.HasBufferedData() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Delaying sending of data because there is still data queued to be send from the previous send" );
            m_sendBuffer.Write( data, 1, length );
            Unlock();
            return true;
        }

        // We will try looping until we have transmitted all the data
        int error = 0;
        UInt32 totalBytesSent = 0;
        Int32 wbytes = 0;
        while ( totalBytesSent < length )
        {
            // perform a send, trying to send as much of the given data as possible
            Int32 remnant = length - totalBytesSent;
            wbytes = dvsocket_send( _data->sockid                 ,
                                    ((Int8*)data)+totalBytesSent  ,
                                    remnant                       ,
                                    0                             ,
                                    &error                        );
            if ( wbytes != SOCKET_ERROR )
            {
                // we where able to send at least some of the data
                totalBytesSent += wbytes;
                m_bytesTransmitted += wbytes;

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Succeeded in sending " + CORE::UInt32ToString( wbytes ) + " bytes of data" );
            }
            else
            {
                // Socket error,..
                // Check if we have to delay sending the data
                if ( DVSOCKET_EWOULDBLOCK == error && !_blocking )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Delaying sending of data" );
                    m_sendBuffer.Write( ((Int8*)data)+totalBytesSent, 1, remnant );
                }
                else
                {
                    m_lastConnFailed = true;
                    Unlock();

                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( error ) );

                    TSocketErrorEventData eData( error );
                    NotifyObservers( SocketErrorEvent, &eData );
                    return false;
                }
            }
        }

        Unlock();
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTCPClientSocket::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return datalock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTCPClientSocket::Unlock( void ) const
{GUCEF_TRACE;

    return datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTCPClientSocket::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString typeName = "GUCEF::COMCORE::CTCPClientSocket";
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::SetUseTcpSendCoalescing( bool coaleseData )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN  )

    if ( _active )
    {
        int errorCode = 0;
        int flag = (coaleseData ? 1 : 0);
        if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(flag), &errorCode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::ToString( this ) + "): Failed to apply data coalescing (TCP_NODELAY) setting \""
                + CORE::ToString( coaleseData ) + "\" to active socket. ErrorCode: " + CORE::ToString( errorCode ) );
            return false;
        }
    }

    m_coaleseDataSends = coaleseData;
    return true;

    #else

    // TCP_NODELAY is not supported in Emscripten/WebAssembly
    // WebSockets don't have the concept of Nagle's algorithm
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPClientSocket(" + CORE::ToString( this ) + "): TCP_NODELAY not supported in WebAssembly environment, skipping" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::GetUseTcpSendCoalescing( void ) const
{GUCEF_TRACE;

    return m_coaleseDataSends;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::SetUseTcpKeepAlive( bool keepAlive )
{
    if ( _active )
    {
        int errorCode = 0;
        int flag = (keepAlive ? 1 : 0);
        if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_KEEPALIVE, (const char*) &flag, sizeof(flag), &errorCode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Failed to apply keep alive (TCP_NODELAY) setting \""
                + CORE::BoolToString( keepAlive ) + "\" to active socket. ErrorCode: " + CORE::Int32ToString( errorCode ) );
            return false;
        }
    }

    m_useTcpKeepAlive = keepAlive;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::GetUseTcpKeepAlive( void ) const
{
    return m_useTcpKeepAlive;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::SetTcpKeepAliveProbeFailMax( UInt8 maxFails )
{
    // @TODO

    //if ( _active )
    //{
    //    int errorCode = 0;
    //    int flag = (int)maxFails;
    //    if ( 0 > ( _data->sockid, IPPROTO_TCP, TCP_KEEPCNT, (const char*) &flag, sizeof(flag), &errorCode ) )
    //    {
    //        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTCPClientSocket(" + CORE::PointerToString( this ) + "): Failed to apply max keep alive probe failures setting \""
    //            + CORE::Int32ToString( flag ) + "\" to active socket. ErrorCode: " + CORE::Int32ToString( errorCode ) );
    //        return false;
    //    }
    //}

    //m_maxKeepAliveProbeFails = maxFails;
    //return true;

    return false;
}

/*-------------------------------------------------------------------------*/

UInt8
CTCPClientSocket::GetTcpKeepAliveProbeFailMax( void ) const
{
    return m_maxKeepAliveProbeFails;
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::SetAutoReconnectOnError( bool autoReconnect )
{GUCEF_TRACE;

    m_autoReconnectOnError = autoReconnect;
}

/*-------------------------------------------------------------------------*/

bool
CTCPClientSocket::GetAutoReconnectOnError( void ) const
{GUCEF_TRACE;

    return m_autoReconnectOnError;
}

/*-------------------------------------------------------------------------*/

void
CTCPClientSocket::SetMaxUpdatesPerCycle( UInt32 maxUpdates )
{GUCEF_TRACE;

    m_maxUpdatesPerCycle = maxUpdates;
}

/*-------------------------------------------------------------------------*/


UInt32
CTCPClientSocket::GetMaxUpdatesPerCycle( void ) const
{GUCEF_TRACE;

    return m_maxUpdatesPerCycle;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
