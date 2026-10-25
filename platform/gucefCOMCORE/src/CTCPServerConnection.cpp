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

#include <string.h>

#include "CTCPServerSocket.h"           /* parent server socket */

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#ifdef ACTIVATE_MEMORY_MANAGER
  #ifndef GUCEF_NEW_ON_H
  #include "gucef_new_on.h"   /* Use the GUCEF memory manager instead of the standard manager ? */
  #define GUCEF_NEW_ON_H
  #endif /* GUCEF_NEW_ON_H ? */
#endif /* ACTIVATE_MEMORY_MANAGER ? */

#include "CTCPServerConnection.h"       /* header for this class */

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    #include <netinet/tcp.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define BUFFER_READ_SIZE	25

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct STCPServerConData
{
    UInt32 sockid;
    CORE::CString clientip;
    struct sockaddr_in clientaddr;
    struct timeval timeout;         /* timeout for blocking operations */
};
typedef struct STCPServerConData TTCPServerConData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CTCPServerConnection::CTCPServerConnection( CTCPServerSocket *tcp_serversock ,
                                            UInt32 connection_idx            )
    : CTCPConnection()                    
    , _data( NULL )                       
    , _blocking( false )                  
    , _active( false )                    
    , m_readbuffer()                      
    , m_sendBuffer()                      
    , m_sendOpBuffer()                    
    , _datalock()                         
    , m_maxreadbytes( 0 )                 
    , m_connectionidx( connection_idx )
    , m_userData( GUCEF_NULL )   
    , m_parentsock( tcp_serversock )      
    , m_coaleseDataSends( true )          
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
    , m_disconnectIfIdle( false )
    , m_idleTimer( tcp_serversock->m_pulseGenerator, 600000 )
{GUCEF_TRACE;

    _data = GUCEF_NEW TTCPServerConData;
    _data->sockid = 0;
    memset( &_data->timeout, 0, sizeof( _data->timeout ) );
    memset( &_data->clientaddr, 0, sizeof(_data->clientaddr) );

    TEventCallback callback1( this, &CTCPServerConnection::OnIdleTimerTriggered );
    SubscribeTo( &m_idleTimer                   ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );
}

/*-------------------------------------------------------------------------*/

CTCPServerConnection::~CTCPServerConnection()
{GUCEF_TRACE;

    Close();

    SignalUpcomingObserverDestruction();
    
    GUCEF_DELETE _data;
    _data = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTCPServerConnection::GetRemoteHostName( void ) const
{GUCEF_TRACE;

    return _data->clientip;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CTCPServerConnection::GetRemoteIP( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return CIPv4Address( _data->clientaddr.sin_addr.S_un.S_addr, _data->clientaddr.sin_port );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return CIPv4Address( _data->clientaddr.sin_addr.s_addr, _data->clientaddr.sin_port );

    #endif
}

/*-------------------------------------------------------------------------*/

CHostAddress
CTCPServerConnection::GetRemoteHostAddress( void ) const
{GUCEF_TRACE;

    return CHostAddress( GetRemoteIP(), GetRemoteHostName() );
}

/*-------------------------------------------------------------------------*/

UInt16
CTCPServerConnection::GetRemoteTCPPort( void ) const
{GUCEF_TRACE;

    return ntohs( _data->clientaddr.sin_port );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTCPServerConnection::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return _datalock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTCPServerConnection::Unlock( void ) const
{GUCEF_TRACE;

    return _datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::Close( void )
{GUCEF_TRACE;

    CloseImp( false, true, true, true );
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::CloseImp( bool byUser            , 
                                bool lock              , 
                                bool updateActiveLists ,
                                bool notify            )
{GUCEF_TRACE;

    if ( lock )
        Lock();

    if ( _active )
    {
        int errorCode = 0;
        if ( 0 == dvsocket_closesocket( _data->sockid, &errorCode ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TCPServerConnection(" + CORE::PointerToString( this ) + "): Closed socket for connection " + GetRemoteIP().AddressAndPortAsString() );
        
            _active = false;

            m_parentsock->OnClientConnectionClosed( this              ,
                                                    m_connectionidx   ,
                                                    byUser            ,
                                                    updateActiveLists );

		    m_readbuffer.Clear( false );
		    m_sendBuffer.Clear( false );
		    m_sendOpBuffer.Clear( false );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TCPServerConnection(" + CORE::PointerToString( this ) + "): Failed close socket at " + GetRemoteIP().AddressAndPortAsString() );
        }
    }

    if ( lock )
        Unlock();

    if ( notify )
        NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Send( const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    return Send( content.GetConstBufferPtr(), content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Send( const void* dataSource ,
                            const UInt32 length    )
{GUCEF_TRACE;

    if ( IsActive() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Sending data of length " << length );

        Lock();

        // notify observers that we are sending data
        CORE::CDynamicBuffer linkedBuffer;
        linkedBuffer.LinkTo( dataSource, length );
        TDataRecievedEventData cData( &linkedBuffer );
        if ( !NotifyObservers( DataSentEvent, &cData ) ) return false;

        // Check if we still have data queued to be sent,..
        // TCP has to be in-order so we will have to queue the new data behind the already queued data
        if ( m_sendBuffer.HasBufferedData() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Delaying sending of data because there is still data queued to be send from the previous send" );
            m_sendBuffer.Write( dataSource, 1, length );
            Unlock();
            return true;
        }

        // We will try looping until we have transmitted all the data
        int error;
        UInt32 totalBytesSent = 0;
        Int32 wbytes;
        while ( totalBytesSent < length )
        {
            // perform a send, trying to send as much of the given data as possible
            Int32 remnant = length - totalBytesSent;
            wbytes = dvsocket_send( _data->sockid                      ,
                                    ((Int8*)dataSource)+totalBytesSent ,
                                    remnant                            ,
                                    0                                  ,
                                    &error                             );
            if ( wbytes != SOCKET_ERROR )
            {
                // we where able to send at least some of the data
                totalBytesSent += wbytes;
                m_bytesTransmitted += wbytes;

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Succeeded in sending " << wbytes << " bytes of data" );
            }
            else
            {
                // Socket error,..
                // Check if we have to delay sending the data
                if ( DVSOCKET_EWOULDBLOCK == error && !_blocking )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Delaying sending of data" );
                    m_sendBuffer.Write( ((Int8*)dataSource)+totalBytesSent, 1, remnant );

                    // The remainder is queued, it will be sent when the socket can take more data.
                    // Without leaving the loop the same remainder would be queued again on every iteration
                    break;
                }
                else
                {
                    Unlock();

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( error ) );

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

void
CTCPServerConnection::SetMaxRead( UInt32 mr )
{GUCEF_TRACE;

    /*
     *      Although the setting of the int value may be atomic we must
     *      use a mutex lock so that a current read proccess will not be
     *      affected. Thus the setting will take effect on the next read
     *      cycle.
     */
    Lock();
    m_maxreadbytes = mr;
    Unlock();
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetMaxRead( void ) const
{GUCEF_TRACE;

    return m_maxreadbytes;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Read( char *dest     ,
                            UInt32 size    ,
                            UInt32 &wbytes ,
                            Int32 timeout  )
{GUCEF_TRACE;
            //@TODO: blocking socket support
     //   _datalock.Lock();
     //   recv( _data->sockid              ,
     //         _readbuffer.GetBufferPtr() ,
     //         _readbuffer.GetSize()      ,
     //         0                          );


     /*
        if ( !datalock.Lock_Mutex() ) return false;
	rb_dest = dest;
        rb_size = size;
        rb_wbytes = &wbytes;
	read_flag = true;
        datalock.Unlock_Mutex();

        return Wait_Untill_Read( timeout );   */
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::CheckRecieveBuffer( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Checking the recieve buffer for incoming data" );

    /*
     *      Since this is a non-blocking socket we need to poll for received data
     */
    int bytesrecv;
    UInt32 totalrecieved( 0 );
    int errorcode;
    UInt32 readblocksize;
    m_maxreadbytes ? readblocksize = m_maxreadbytes : readblocksize = 1024;
    do
    {
        m_readbuffer.SetDataSize( m_readbuffer.GetDataSize()+readblocksize );
        bytesrecv = dvsocket_recv( _data->sockid                                                 ,
                                   static_cast<char*>(m_readbuffer.GetBufferPtr())+totalrecieved ,
                                   readblocksize                                                 ,
                                   0                                                             ,
                                   &errorcode                                                    );

        // A non-blocking socket which has been read empty reports that nothing more can be read right now,
        // which is the normal end of a read cycle when the amount of data was an exact multiple of the block size
        if ( bytesrecv == SOCKET_ERROR && DVSOCKET_EWOULDBLOCK == errorcode && !_blocking )
        {
            // the buffer was grown to receive more, only what we actually got is valid
            m_readbuffer.SetDataSize( totalrecieved );
            break;
        }

        // Check for an error
        if ( ( bytesrecv == SOCKET_ERROR ) || ( errorcode != 0 ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorcode ) );

            // Notify our users of the error
            TSocketErrorEventData eData( errorcode );
            if ( !NotifyObservers( SocketErrorEvent, &eData ) )
                return false;

            CloseImp( true, false, true, false );
            return true;
        }

        if ( (  0 == totalrecieved ) &&
             (  0 == bytesrecv     )  )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Client closed the connection" );

            /*
             *      we arrived here because the read flag was set, however no data is available
             *      This means that the client closed the connection
             */
            CloseImp( true, false, true, true );
            return true;
        }
        totalrecieved += bytesrecv;
        m_readbuffer.SetDataSize( totalrecieved );

        if ( m_maxreadbytes > 0 )
        {
            if ( m_maxreadbytes <= totalrecieved )
            {
                // This is a busy socket, don't yield to the scheduler
                m_parentsock->m_pulseGenerator->RequestImmediatePulse();
                break;
            }
        }
    }
    while ( bytesrecv == readblocksize );

    if ( totalrecieved > 0 )
    {
        m_idleTimer.Reset();
        m_bytesReceived += totalrecieved;

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Received " << totalrecieved << " bytes of data" );
        TDataRecievedEventData eData( &m_readbuffer );

        // false means this object was deleted by one of the observers, we must not touch it anymore
        return NotifyObservers( DataRecievedEvent, &eData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetConnectionIndex( void ) const
{GUCEF_TRACE;

    return m_connectionidx;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::OnIdleTimerTriggered( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Idle timer tiggered. Closing connection due to inactivity" );
    Close();
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerConnection::SetDisconnectIfIdle( bool disconnectIfIdle )
{GUCEF_TRACE;

    if ( m_disconnectIfIdle != disconnectIfIdle )
    {
        m_disconnectIfIdle = disconnectIfIdle;
        m_idleTimer.SetEnabled( m_disconnectIfIdle );
    }
}

/*-------------------------------------------------------------------------*/

bool 
CTCPServerConnection::GetDisconnectIfIdle( void ) const
{GUCEF_TRACE;

    return m_disconnectIfIdle;
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerConnection::SetMaxIdleDurationInMs( UInt32 maxIdleTimeInMs )
{GUCEF_TRACE;

    m_idleTimer.SetInterval( maxIdleTimeInMs );
}

/*-------------------------------------------------------------------------*/

UInt32 
CTCPServerConnection::GetMaxIdleDurationInMs( void ) const
{GUCEF_TRACE;

    return m_idleTimer.GetInterval();
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Update( UInt32 maxUpdatesPerCycle )
{GUCEF_TRACE;

    if ( !_blocking && _active )
    {
        fd_set readfds;      /* Setup the read variable for the select function */
        fd_set exceptfds;    /* Setup the except variable for the select function */

        FD_ZERO( &readfds );
        FD_ZERO( &exceptfds );

        Lock();

        FD_SET( _data->sockid, &readfds );
        FD_SET( _data->sockid, &exceptfds );

        int errorCode = 0;
        if ( dvsocket_select( _data->sockid+1   ,
                              &readfds          ,
                              NULL              , // We don't care about socket writes here
                              &exceptfds        ,
                              &_data->timeout   ,
                              &errorCode        ) != SOCKET_ERROR )
        {
            /* something happened on the socket */

            if ( FD_ISSET( _data->sockid, &exceptfds ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorCode ) );

                TSocketErrorEventData eData( errorCode );
                if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return false;

                CloseImp( true, false, true, false );
                
                Unlock();
                return true;
            }
            else
            if ( FD_ISSET( _data->sockid, &readfds ) )
            {
                /* data can be read from the socket */
                // false means this object was deleted by one of the observers, we must not touch it anymore
                if ( !CheckRecieveBuffer() )
                    return false;
            }
        }
        else
        {
            /* select call failed */
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured (select call failed): " + CORE::Int32ToString( errorCode ) );

            TSocketErrorEventData eData( errorCode );
            NotifyObservers( SocketErrorEvent, &eData );

            CloseImp( false, false, true, false );
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
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Succeeded in delayed sending " << wbytes << "bytes of queued data" );
                    }
                    else
                    {
                        // Socket error,..
                        // Check if we have to delay sending the data
                        if ( DVSOCKET_EWOULDBLOCK == error && !_blocking )
                        {
                            // Cannot send now,... try again next pulse
                            // We have to place remaining data we grabbed from the send buffer back in
                            // the send buffer in a FILO manner
                            m_sendBuffer.WriteAtFrontOfQueue( data+totalBytesSent ,
                                                              remnant             ,
                                                              1                   );

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Unable to delayed send queued data at this time" );

                            Unlock();
                            return true;
                        }
                        else
                        {
                            Unlock();

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( error ) );

                            TSocketErrorEventData eData( error );
                            NotifyObservers( SocketErrorEvent, &eData );
                            return true;
                        }
                    }
                }
            }
        }

        Unlock();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::SetUserData( void* userData )
{GUCEF_TRACE;

    m_userData = userData;
}

/*-------------------------------------------------------------------------*/

void*
CTCPServerConnection::GetUserData( void ) const
{GUCEF_TRACE;

    return m_userData;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetBytesReceived( bool resetCounter )
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
CTCPServerConnection::GetBytesTransmitted( bool resetCounter )
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
CTCPServerConnection::IsActive( void ) const
{GUCEF_TRACE;

    return _active;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::SetUseTcpSendCoalescing( bool coaleseData )
{
    if ( _active )
    {
        int flag = (coaleseData ? 1 : 0);
        int errorCode = 0;
        if ( -1 == dvsocket_setsockopt( _data->sockid, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag), &errorCode ) )
        {
            return false;
        }
    }

    m_coaleseDataSends = coaleseData;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::GetUseTcpSendCoalescing( void ) const
{
    return m_coaleseDataSends;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
